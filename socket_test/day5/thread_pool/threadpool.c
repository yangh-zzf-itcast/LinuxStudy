/*************************************************************************
    > File Name: threadpool.c
    > Author: yanghang
    > Mail: 2459846416@qq.com 
    > Created Time: Tue 25 Dec 2018 08:49:30 PM CST
 ************************************************************************/

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<pthread.h>
#include<sys/stat.h>

#define DEFAULT_TIME 10				/* 10s 检测一次 */
#define	MIN_WAIT_TASK_NUM 10		/* 如果queue_size > MIN_WAIT_TASK_NUM，添加新的线程到线程池 */
#define DEFAULT_THREAD_VARY 10		/* 每次创建和销毁的线程的个数 */
#define true 1
#define false 0

/* 线程任务结构体 */
typedef struct{
	void *(*function)(void *);		/* 函数指针作函数参数， 回调函数 */
	void *arg;						/* 回调函数的参数 */
}threadpool_task_t;					/* 各子线程任务结构体 */

/* 线程池结构体 */
struct threadpool_t{
	/*互斥锁*/
	pthread_mutex_t lock;			/* 用于锁住本结构体 */
	pthread_mutex_t thread_counter;	/* 记录忙碌状态线程个数的锁 -- busy_thr_num */
	/*条件变量*/
	pthread_cond_t queue_not_full;	/* 当任务队列满时，添加任务的线程阻塞，等待此条件变量，服务器阻塞在此 */
	pthread_cond_t queue_not_empty;	/* 任务队列里不为空时，通知等待任务的线程，线程阻塞在此 */

	pthread_t *threads;				/* 存放线程池中的每个线程的tid 数组 */
	pthread_t adjust_tid;			/* 存管理线程tid 管理线程用来管理线程池 */
	threadpool_task_t *task_queue;	/* 任务队列（数组的首地址） */

	int min_thr_num;				/* 线程池最小线程数 */
	int max_thr_num;				/* 线程池最大线程数 */
	int live_thr_num;				/* 当前存活的线程个数 */
	int busy_thr_num;				/* 忙状态线程个数 */
	int wait_exit_thr_num;			/* 要销毁的线程个数 */
									/* live线程个数与busy线程个数的比例达到某个值，adjust_tid 管理线程对 线程池进行瘦身或者扩容*/

	int queue_front;				/* task_queue 队头下标 */
	int queue_rear;					/* task_queue 队尾下标 */
	int queue_size;					/* task_queue 队中实际任务数 */
	int queue_max_size;				/* task_queue队列可容忍任务数上限 */

	int shutdown;					/* 标志位，线程池的使用状态，true或者false */
};

void *threadpool_thread(void *threadpool);

void *adjust_thread(void *threadpool);

int is_thread_alive(pthread_t tid);
int threadpool_free(threadpool_t *pool);

/* 创建一个线程池 也就是为 线程池结构体 内的变量初始化的过程 */
threadpool_t * threadpool_create(int min_thr_num, int max_thr_num, int queue_max_size)
{
	int i;
	threadpool_t *pool = NULL;		//创建一个pool结构体

	do{
		if((pool = (threadpool_t *)malloc(sizeof(threadpool_t))) == NULL){
			printf("malloc threadpool fail");
			break;					//跳出循环
		}

		/* 对结构体变量 初始化*/
		pool->min_thr_num = min_thr_num;
		pool->max_thr_num = max_thr_num;
		pool->busy_thr_num = 0;
		pool->live_thr_num = min_thr_num;		/* 存活的线程数 初始值=最小线程数*/
		pool->wait_exit_thr_num = 0;
		pool->queue_size = 0;					/* 有0个产品 任务数*/
		pool->queue_max_size = queue_max_size;  /* 最大任务队列数 */
		pool->queue_front = 0;					/* 任务队列的队头指针front 和队尾指针rear 都指向起始位置*/
		pool->queue_rear = 0;
		pool->shutdown = false;					/* 打开线程池 */

		/* 根据最大线程上限数，给工作线程数组开辟空间 并初始化清零 */
		pool->threads = (pthread_t *)malloc(sizeof(pthread_t) * max_thr_num);
		if(pool->threads == NULL){
			printf("malloc threads fail");
			break;
		}
		memset(pool->threads, 0, sizeof(pthread_t) * max_thr_num);

		/* 给任务队列开辟空间 */
		pool->task_queue = (thread_task_t *)malloc(sizeof(thread_task_t) * queue_max_size);
		if(pool->task_queue == NULL){
			printf("malloc task queue fail");
			break;
		}

		/* 互斥锁 与 条件变量 的初始化*/
		if(pthread_mutex_init(&(pool->lock), NULL) != 0
			|| pthread_mutex_init(&(pool->thread_counter), NULL) != 0
			|| pthread_cond_init(&(pool->queue_not_empty), NULL) != 0
			|| pthread_cond_init(&(pool->queue_not_full), NULL) != 0)
		{
			printf("init the lock and cond fail");
			break;
		}

		/* 启动 min_thr_num 个 work thread */
		for(i = 0;i < min_thr_num;i++){
			/* pool 指向当前线程池 ； threadpool_thread 是回调函数*/
			/* 所有子线程 阻塞  在回调函数内 的 条件变量上 */
			pthread_create(&(pool->threads[i]), NULL, threadpool_thread, (void *)pool);
			printf("start thread 0x%x...\n", (unsigned int)pool->threads[i]);
		}
		
		/* 启动管理者线程 用来回调函数 adjust_thread 管理线程池, 回调函数的参数就是 线程池结构体*/
		pthread_create(&(pool->adjust_tid), NULL, adjust_thread, (void *)pool);

		return pool;
	}while(0);

	threadpool_free(pool);		/* 前面步骤出错跳出时，释放线程池pool存储空间 */

	return NULL;
}

/* 线程池中 各个工作线程 */
void *threadpool_thread(void *threadpool)
{
	threadpool_t *pool = (threadpool_t *)threadpool;
	threadpool_task_t task;		/* 任务 */

	while(true){
		/* 刚创建出的线程，等待任务队列里有任务；否则阻塞等待任务队列里有任务，再唤醒线程 接收任务 */
		pthread_mutex_lock(&(pool->lock));	/* 锁整个 pool 结构体 */

		/* queue_size == 0 说明队列中无任务，调用wait阻塞在条件变量上；若有任务，跳过该while语句 */
		while((pool->queue_size == 0) && (!pool->shutdown)){
			printf("thread 0x%x is waiting\n", (unsigned int)pthread_self());
			pthread_cond_wait(&(pool->queue_not_empty), &(pool->lock));

			/* 清楚指定数组的空闲线程，如果要结束的线程个数大于0，结束这些线程 */
			if(pool->wait_exit_thr_num > 0){
				pool->wait_exit_thr_num--;		/* 要退出的线程数 -- */

				/* 如果线程池里线程个数大于最小值时，可以结束当前子线程；否则不行 */
				if(pool->wait_exit_thr_num > pool->min_thr_num){
					printf("thread 0x%x is exiting\n", (unsigned int)pthread_self());
					pool->live_thr_num--;		/* 存活的线程数 -- */
					pthread_mutex_unlock(&(pool->lock));

					pthread_exit(NULL);			/* 该子线程退出 */
				}
			}
		}

		/* 如果指定了trye ，要关闭线程池里的每一个线程，自行退出处理 ----销毁线程池 */
		if(pool->shutdown){
			pthread_mutex_unlock(&(pool->lock));
			printf("thread 0x%x, is exiting\n", (unsigned int)pthread_self());
			pthread_detach(pthread_self());		/* 设置 子线程 与 主线程 分离 */
			pthread_exit(NULL);					/* 线程自行结束 */
		}
		
		/* 从任务队列里获取任务，是一个出队操作 */
		task.function = pool->task_queue[pool->queue_front].function;	
		task.arg = pool->task_queue[pool->queue_front].arg;		/* 队头指针，先进先出 */

		pool->queue_front = (pool->queue_front + 1) % pool->queue_max_size;	/* 出队 */
		pool->queue_size--;

		/* 通知可以有新的任务添加进来 */
		pthread_cond_broadcast(&(pool->queue_not_full));

		/* 任务取出后， 立即将线程池 锁释放 */
		pthread_mutes_unlock(&(pool->lock));

		/* 执行任务！*/
		printf("pthread 0x%x start working \n", (unsigned int)pthread_self());
		pthread_mutex_lock(&(pool->thread_counter));
		pool->busy_thr_num++;
		pthread_mutex_unlock(&(pool->thread_counter));	/* 添加1个 忙碌状态 线程数 +1 */

		/* 就是 sleep（1）的步骤 ，arg 就是 num[i] */
		(*(task.function))(task.arg);					/* 执行回调函数任务 */

		/* 任务结束 后的处理步骤 */
		printf("thread 0x%x end working\n", (unsigned int)pthread_self());
		pthread_mutex_lock(&(pool->thread_countrt));
		pool->busy_thr_num--;							/* 处理完1个 忙绿状态的线程 线程数 -1 */
		pthread_mutex_unlock(&(pool->thread_counter));
	}

	pthread_exit(NULL);
}

/* 管理者线程 */
void *adjust_thread(void *threadpool)
{
	int i;
	threadpool_t *pool = (threadpool_t)threadpool;

	while(!pool->shutdown){
		sleep(DEFAULT_TIME);		/* 定时 对线程池管理 ，宏定义 10s */
		
		pthread_mutex_lock(&(pool->lock));
		int queue_size = pool->queue_size;		/* 关注 任务数 */
		int live_thr_num = pool->live_thr_num;	/* 存活 线程数 */
		pthread_mutex_unlock(&(pool->lock));

		pthread_mutex_lock(&(pool->thread_counter));
		int busy_thr_num = pool->busy_thr_num;	/* 忙碌 线程数 */
		pthread_mutex_unlock(&(pool->thread_counter));

		/* 创建新线程算法： 任务数大于最小线程池个数 且存活的线程数少于最大线程个数时，进行创建*/
		/* 这个比例可以自行调节 */
		if(queue_size > MIN_WAIT_TASK_NUM && live_thr_num < pool->max_thr_num){
			pthread_mutex_lock(&(pool->lock));
			int add = 0;

			/* 一次增加指定步长 DEFAULT_THREAD_VARY 个线程 */
			for(i = 0;i < pool->max_thr_num && add < DEFAULT_THREAD_VARY
					&& pool->live_thr_num < pool->max_thr_num; i++){
				if(pool->threads[i] == 0 || !is_thread_alive(pool->threads[i])){
					/* 创建一个子线程成功 */
					pthread_create(&(pool->thread[i]), NULL, threadpool_thread, (void *)arg);
					add++;
					pool->live_thr_num++;
				}
			}

			pthread_mutex_unlock(&(pool->lock));
		}

		/* 销毁多余的空间线程 算法：忙线程*2 < 存活的线程数 且 存活的线程数大于 最小线程数 */
		/* 比例可自行调节 */
		if(busy_thr_num * 2 < live_thr_num && live_thr_num > pool->min_thr_num){
			/* 一次销毁 DEFAULT_THREAD_VARY 个线程，随机 */
			pthread_mutex_lock(&(pool->lock));
			pool->wait_exit_thr_num = DEFAULT_THREAD_VARY;
			pthread_mutex_unlock(&(pool->lock));

			for(i = 0;i < DEFAULT_THREAD_VARY;i++){
				/* 骗有任务进来，通知处于空闲状态的线程， 他们会自行终止 */
				/* 在threadpool_thread 中 执行 thread_exit(NULL) */
				pthread_cond_signal(&(pool->queue_not_empty));
			}
		}
	}

	return NULL;
}

/* 向线程池任务队列中 添加任务，任务由回调函数实现 */
int threadpool_add(threadpool_t thp, void *(*function)(void *arg), void *arg)
{
	pthread_mutex_lock(&(pool->lock));

	/* 队列中的实际任务数达到最大，造成阻塞 */
	while((pool->queue_size == pool->queue_max_size) && (!pool->shutdown)){
		pthread_cond_wait(&(pool->queue_not_full), &(pool->lock));
	}

	if(pool->shutdown){
		pthread_cond_broadcast(&(pool->queue_not_empty));	//唤醒全部线程
		pthread_mutex_unlock(&(pool->lock));
		return 0;
	}

	/* 清空 工作线程 调用的回调函数的 参数arg */
	if(pool->task_queue[pool->queue_rear].arg != NULL){
		pool->task_queue[pool->queue_rear].arg = NULL;
	}

	/* 设置新任务的回调函数和参数，然后添加新的任务到任务队列中 */
	pool->task_queue[pool->queue_rear].function = function;
	pool->task_queue[pool->queue_rear].arg = arg;
	/* 队尾指针移动 模拟环形队列 */
	pool->queue_rear = (pool->queue_rear +1) % pool->queue_max_size;

	pool->queue_size++;		/* 任务队列中实际任务数 + 1 */

	/* 添加完任务后，队列不为空，*/
	pthread_cond_signal(&(pool->queue_not_empty));
	pthread_mutex_unlock(&(pool->lock));

	return 0;
}

/* 线程的销毁 */
int threadpool_destroy(threadpool_t *pool)
{
	int i;
	if(pool == NULL)
		return -1;

	pool->shutdown = true;

	/* 先销毁管理线程 */
	pthread_join(pool->adjust_tid, NULL);

	/* 所有线程都是这样被骗自杀 */
	for(i = 0;i < pool->live_thr_num;i++){
		/* 假装有任务，通知 所有 的空闲线程，唤醒所有线程，后面执行 自行退出 */
		pthread_cond_broadcast(&(pool->queue_not_empty));
	}

	/* 回收所有线程的资源 */
	for(i = 0;i < pool->live_thr_num;i++){
		pthread_join(pool->threads[i],NULL);
	}

	return 0;
}

int threadpool_all_threadnum(threadpool_t *pool)
{
	int all_threadnum = -1;

	pthread_mutex_lock(&(pool->lock));
	all_threadnum = pool->live_thr_num;		/* 存活线程 个数 */

	return all_threadnum;
}

/* 释放 pool 线程池结构体 的空间*/
int threadpool_free(threadpool_t *pool)
{
	if(pool == NULL)
		return -1;
	
	if(pool->task_queue){
		free(pool->task_queue);
	}

	if(pool->pthreads){
		free(pool->threads);
		pthread_mutex_lock(&(pool->lock));
		pthread_mutex_destroy(&(pool->lock));

		pthread_mutex_lock(&(pool->thread_counter));
		pthread_mutex_destroy(&(pool->thread_counter));

		pthread_cond_destroy(&(pool->queue_not_empty));
		pthread_cond_destroy(&(pool->queue_not_full));
	}

	free(pool);
	pool = NULL;

	return 0;
}

/* 线程池中的线程 ， 模拟处理业务 */
void *process(void *arg)
{
	printf("thread 0x%x working on task%d\n", (unsigned int)pthread_self(), (int)arg);
	
	/**** 任务代码 *****/
	sleep(1);		/* 模拟 小写 ----> 大写*/
	printf("task %d is end\n", (int)arg);
	/**** 任务结束 *****/
	
	return NULL;
}

int main(void)
{
	/* threadpool_t * threadpool_create(int min_thr_num, int max_thr_num, int queue_max_size) */
	threadpool_t *thp = threadpool_create(3, 100, 100);	/*创建线程池，池里最小3个线程，最大100个线程，任务队列最大100个 */
	printf("pool inited");

	//int *num = (int *)malloc(sizeof(int)*20);
	int num[20], i;
	for(i = 0;i < 20;i++){
		num[i] = i;
		printf("add task %d\n", i);

		/* int threadpool_add(threadpool_t *pool, void *(*function)(void *arg), void *arg) */
		threadpool_add(thp, process, (void *)&num[i]);		/* 向线程池中添加任务 */
															/* 给线程池中的线程 分配任务*/
	}

	sleep(10);								/* 等子线程完成任务 */
	threadpool_destroy(thp);				/* 销毁线程池 */

	return 0;
}
