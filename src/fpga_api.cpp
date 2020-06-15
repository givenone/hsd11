#include "fpga_api.h"
#include <cstdio>
#include <cstring>

#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

#define DATA_SIZE SIZE*(SIZE+1)*sizeof(float) // fpga bram data size

#define min(x,y) (((x)<(y))?(x):(y))

FPGA::FPGA(off_t data_addr, off_t api_addr)
{
    fd_ = open("/dev/mem", O_RDWR);
    data_ = static_cast<float*>(mmap(NULL, DATA_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, fd_, data_addr));
    api_ = static_cast<unsigned int*>(mmap(NULL, sizeof(unsigned int), PROT_READ|PROT_WRITE, MAP_SHARED,fd_, api_addr));
}

FPGA::~FPGA()
{
    munmap(data_, DATA_SIZE );
    munmap(api_, sizeof(unsigned int));
    close(fd_);
}

float* FPGA::matrix(void)
{
	return data_ + SIZE;
}

float* FPGA::vector(void)
{
	return data_;
}

const float* __attribute__((optimize("O0"))) FPGA::run()
{
    *api_ = 0x5555;
    while(*api_ == 0x5555);

    return data_;    
}

// Test code for bitstream
void FPGA::largeMV(const float* large_mat, const float* input,
		float* output, int num_input, int num_output)
{
  float *vec = this->vector();
  float *mat = this->matrix();

  // 0) Initialize output vector
  for (int i = 0; i < num_output; ++i)
    output[i] = 0;

  for (int i = 0; i < num_output; i += m_size_)
  {
    for (int j = 0; j < num_input; j += v_size_)
    {
        // 0) Initialize input vector		
        int block_row = min(m_size_, num_output-i);
        int block_col = min(v_size_, num_input-j);

        // !) Assign a vector
        /* IMPLEMENT */
        memcpy(vec, input + j, sizeof(float) * block_col);
        //if(block_col < v_size_) memset()
        // 2) Assign a matrix
        /* IMPLEMENT */
        int k=0;
        for(; k< block_row ; k++)
        {
            memcpy(mat+ v_size_* k, large_mat + (i+k) * num_input + j, sizeof(float) * block_col);
            if(block_col < v_size_) memset(mat+ v_size_ * k + block_col, 0, sizeof(float) * (v_size_ - block_col));
        }
        if(k < m_size_)
        {
            for(int x = 0; x < m_size_ - k ; x++)
            {
                memset(mat+ v_size_ * ( k + x ), 0, sizeof(float) * v_size_);
            }
        }
        // 3) Call a function `block_call() to execute MV multiplication
        const float* ret = this->blockMV();

        // 4) Accumulate intermediate results
        for(int row = 0; row < block_row; ++row)
        {
            output[i + row] += ret[row];
        }
    }
  }
}
