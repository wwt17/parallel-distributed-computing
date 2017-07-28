Q: MPI属于什么编程模型？

A: MIMD。因为同一份代码会被复制到很多进程中执行，这些进程都是并发的。

Q: OpenCL里的workgroup是什么？他在实际GPU硬件上的对应物是什么？

A: OpenCL里的workgroup是一些workitems的集合，这些workitems共享local-memory，并且可以进行同步。workgroup就相当于CUDA里的block，在GPU上对应于SM(Streaming Multiprocessor)。

Q: 数据从CPU拷贝到GPU需要经过怎样的流程？

A: 对于host上已有的一段空间，需要在GPU上分配相应的一段空间。然后将host上的空间内的数据（同步/异步）传输到GPU上。为了加速，可以在host上使用pinned memory从而建立直接的物理映射。

Q: 程序调优的基本思路是？

A: 我不清楚这个问题具体指什么。如果泛泛而谈的话，我们需要能够测量系统中每一部分所占用资源大小的情况。尝试其中成为瓶颈的部分。以及我们还可以用复杂度等方式来估算程序的运行效率。
