#include "reader.h"
#include "bufferpool.h"
#include "queue.h"
#include "framebuffer.h"

Reader::Reader(std::string filepath,
               std::string dataset,
               unsigned int dimX,
               unsigned int dimY,
               unsigned int frames,
               unsigned int framesPerBuffer,
               BufferPool *pool,
               Queue<FrameBuffer *> *destQueue)
: m_filepath(filepath),
  m_dataset(dataset),
  m_dimX(dimX),
  m_dimY(dimY),
  m_frames(frames),
  m_framesPerBuffer(framesPerBuffer),
  m_pool(pool),
  m_destQueue(destQueue)
{
  setup();
}

Reader::~Reader()
{

}

void Reader::setup()
{
  
  m_dims[0] = m_frames;
  m_dims[1] = m_dimX;
  m_dims[2] = m_dimY;

  // Chunking size
  m_chunk[0] = 1;
  m_chunk[1] = m_dims[1];
  m_chunk[2] = m_dims[2];

  // Size of hyperslab. 
  m_count[0] = m_framesPerBuffer;
  m_count[1] = m_dims[1];
  m_count[2] = m_dims[2];

  hid_t fileID = H5Fopen(m_filepath.c_str(), H5F_ACC_RDONLY, H5P_DEFAULT);
  m_datasetID = H5Dopen1(fileID, m_dataset.c_str());
  m_spaceID = H5Dget_space(m_datasetID);
  m_memspaceID = H5Screate_simple(3, m_count, NULL);

}

void Reader::run()
{

  hsize_t offset[3];
  offset[0] = offset[1] = offset[2] = 0;

  hid_t HDF5_datatype;
  HDF5_datatype = H5T_NATIVE_UINT16;

  for (offset[0] = 0 ; offset[0] < m_frames; offset[0] += m_count[0])
  {
    // Might block if there is no buffer in the pool. 
    FrameBuffer *buffer = m_pool->take();
    unsigned short *valuebuffer = buffer->getValueBuffer();
    
    herr_t errstatus = H5Sselect_hyperslab(m_spaceID, H5S_SELECT_SET, offset, NULL, m_count, NULL);
    hid_t status = H5Dread(m_datasetID, HDF5_datatype, m_memspaceID, m_spaceID, H5P_DEFAULT, valuebuffer);

    // printf("(Reader) Frame = %d\n", offset[0]);
    // for ( int i = 0 ; i < 10; i++)
    // {
    //   printf("%d ", valuebuffer[i]);
    // }

    // printf("\n");
    
    buffer->setStart(0);
    buffer->setSize(m_dims[1] * m_dims[2] * m_count[0]);
    
    m_destQueue->push(buffer);
  }
}