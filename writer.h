#ifndef WRITER_H
#define WRITER_H

#include "thread.h"

template <typename T>
class Queue;

class BufferPool;
class FrameBuffer;

class Writer : public Thread
{

public:

  Writer(std::string filepath, 
        Queue<FrameBuffer *> *srcQueue,
        BufferPool *bufferPool);

  ~Writer();

protected:

  void run();

private:

  std::string m_filepath;

  Queue<FrameBuffer *> *m_srcQueue;

  BufferPool *m_bufferPool;

  int m_fd;

  void setup();

};
#endif