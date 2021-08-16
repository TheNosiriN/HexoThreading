# Hexo Threading
Tiny Header-only Threading Library (Alpha). Made in C++14


## Usage

Every function is called by from the Threading Engine.

```c++
using namespace Hexo;

ThreadingEngine hxt = ThreadingEngine();
```

\
All spawned threads are tracked and linked with the life of ThreadingEngine through a `Hexo::ResourceList`. But the Thread pools are not tracked and should be destroyed manually.



**There are 5 spawnable components in the library**

****
- **Immediate Threads**: They are used as normal threads. They take in data and function to run at construction. They can be joined or left alone to run asynchronously. They destroy when the task is finished.

```c++
int num = 1645;

HXImmediateThread t = hxt.SpawnImmediateThread(num,
  [&num](void* data){
    num *= 10;
  }
);

hxt.JoinThread(t);
std::cout << "Immediate thread: " << num << '\n';
```


****
- **Worker Threads**: They are threads that live until explicitly destroyed. They take in data and function to run at every task you submit. They cannot be joined but they can be waited on by watching it's taskQueue size.

```c++
HXWorkerThread t = hxt.SpawnWorkerThread();

for (int i=0; i<10; i++){
  int num = 1645 + i;

  hxt.SubmitTask(t, num,
    [](void* data){
      int num = *(reinterpret_cast<int*>(data)) * 10;
      std::cout << "Worker thread: " << num << '\n';
    }
  );
}

//wait for worker thread to callback
while (true){
  volatile size_t s = hxt.GetQueueSize(t);
  if (!s)break;
}


hxt.DestroyThread(t);
```


****
- **Dedicated Threads**: They are threads that live until explicitly destroyed. They run on just one function which is specified at construction. They take in only data at every task you submit. They cannot be joined but they can be waited on by watching it's taskQueue size.

```c++
HXDedicatedThread t = hxt.SpawnDedicatedThread(
  [](void* data){
    int num = *(reinterpret_cast<int*>(data)) * 10;
    std::cout << "Dedicated thread: " << num << '\n';
  }
);

for (int i=0; i<10; i++){
  int num = 1645 + i;
  hxt.SubmitTask(t, num);
}

//wait for dedicated thread to callback
while (true){
  volatile size_t s = hxt.GetQueueSize(t);
  if (!s)break;
}


hxt.DestroyThread(t);
```


****
- **Worker Thread Pools**: They are a pool of Worker Threads. They are given their number of threads at construction and take in the same parameters at Worker Threads at every task. They cannot be joined but they can be waited on by watching it's taskQueue size.

```c++
HXWorkerThreadPool t = hxt.SpawnWorkerPool(4);

int* numarray = new int[10];

for (int i=0; i<10; i++){
  numarray[i] = 1645 + i;

  hxt.SubmitTask(t, i,
    [&numarray](void* data){
      int i = *(reinterpret_cast<int*>(data));
      numarray[i] *= 10;
    }
  );
}

//wait for worker thread pool to finish
while (true){
  volatile size_t s = hxt.GetQueueSize(t);
  if (!s)break;
}

hxt.DestroyPool(t);

for (size_t i = 0; i < 10; i++) {
  std::cout << "Worker pool: " << numarray[i] << '\n';
}
delete[] numarray;
```


****
- **Dedicated Thread Pools**: They are a pool of Dedicated Threads. They are given their number of threads at construction and take in the same parameters at Dedicated Threads at every task. They cannot be joined but they can be waited on by watching it's taskQueue size.

```c++
HXDedicatedThreadPool t = hxt.SpawnDedicatedPool(4,
  [](void* data){
    **(reinterpret_cast<int**>(data)) *= 10;
  }
);

int* numarray = new int[10];

for (int i=0; i<10; i++){
  numarray[i] = 1645 + i;
  int* ptr = numarray+i;
  hxt.SubmitTask(t, ptr);
}

//wait for dedicated thread pool to finish
while (true){
  volatile size_t s = hxt.GetQueueSize(t);
  if (!s)break;
}

hxt.DestroyPool(t);

for (size_t i = 0; i < 10; i++) {
  std::cout << "Dedicated pool: " << numarray[i] << '\n';
}
delete[] numarray;
```


****
## Future Plans
This library is still in its early alpha stages. It was originally planned to be part of the Hexo Engine, and will still be part, but now I'm making this independently along with some other modules of Hexo. I will continue developing it and adding more functionality and optimizations in the future.
