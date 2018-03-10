#pragma once

#include <d3d12.h>

// D3D12 timer.
class D3D12Timer {
    public:
        // Constructor.
        D3D12Timer(ID3D12Device* pDevice)
        {
            mpDevice = pDevice;

            mActive = false;
            mDeltaTime = 0;
            mBeginTime = 0;
            mEndTime = 0;
            mQueryCount = 2;

            D3D12_QUERY_HEAP_DESC queryHeapDesc;
            queryHeapDesc.Type = D3D12_QUERY_HEAP_TYPE_TIMESTAMP;
            queryHeapDesc.NodeMask = 0;
            queryHeapDesc.Count = mQueryCount;

			mpDevice->CreateQueryHeap(&queryHeapDesc, IID_PPV_ARGS(&mQueryHeap));
        
            {
                D3D12_RESOURCE_DESC resouceDesc;
                ZeroMemory(&resouceDesc, sizeof(resouceDesc));
                resouceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
                resouceDesc.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
                resouceDesc.Width = sizeof(UINT64) * mQueryCount;
                resouceDesc.Height = 1;
                resouceDesc.DepthOrArraySize = 1;
                resouceDesc.MipLevels = 1;
                resouceDesc.Format = DXGI_FORMAT_UNKNOWN;
                resouceDesc.SampleDesc.Count = 1;
                resouceDesc.SampleDesc.Quality = 0;
                resouceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
                resouceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

				D3D12_HEAP_PROPERTIES heapProp = {};
				heapProp.Type = D3D12_HEAP_TYPE_READBACK;
				heapProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
				heapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
				heapProp.CreationNodeMask = 1;
				heapProp.VisibleNodeMask = 1;

				if (SUCCEEDED(mpDevice->CreateCommittedResource(
					&heapProp,
					D3D12_HEAP_FLAG_NONE,
					&resouceDesc,
					D3D12_RESOURCE_STATE_COPY_DEST,
					nullptr,
					IID_PPV_ARGS(&mQueryResource))
				))
				{
					mQueryResource->SetName(L"Query result");
				}
            }
        }

        // Destructor.
        ~D3D12Timer()
        {
            mQueryHeap->Release();
            mQueryResource->Release();
        }

        // Start timestamp.
        void Start(ID3D12GraphicsCommandList* pCommandList)
        {
            //assert(!mActive);
            mActive = true;

            pCommandList->EndQuery(mQueryHeap, D3D12_QUERY_TYPE_TIMESTAMP, 0);
        }

        // Stop timestamp.
        void Stop(ID3D12GraphicsCommandList* pCommandList)
        {
            //assert(mActive);
            mActive = false;

            pCommandList->EndQuery(mQueryHeap, D3D12_QUERY_TYPE_TIMESTAMP, 1);
        }

        // Resolve query data. Write query to device memory. Make sure to wait for query to finsih before resolving data.
        void ResolveQuery(ID3D12GraphicsCommandList* pCommandList)
        {
            pCommandList->ResolveQueryData(mQueryHeap, D3D12_QUERY_TYPE_TIMESTAMP, 0, mQueryCount, mQueryResource, 0);
        }

        // Calcluate time and map memory to CPU.
        void CalculateTime()
        {
            // Copy to CPU.
            UINT64 timeStamps[2];
            {
                void* mappedResource;
				D3D12_RANGE readRange{ 0, sizeof(UINT64) * mQueryCount };
				D3D12_RANGE writeRange{ 0, 0 };
				if (SUCCEEDED(mQueryResource->Map(0, &readRange, &mappedResource)))
				{
					memcpy(&timeStamps, mappedResource, sizeof(UINT64) * mQueryCount);
					mQueryResource->Unmap(0, &writeRange);
				}
            }

            mBeginTime = timeStamps[0];
            mEndTime = timeStamps[1];

//			if (mBeginTime != 0) MessageBoxA(0, "ddd", "", 0);

			mDeltaTime = mEndTime - mBeginTime;
        }

        // Get time from start to stop in nano seconds.
        UINT64 GetDeltaTime()
        {
            return mDeltaTime;
        }

		UINT64 GetEndTime()
		{
			return mEndTime;
		}

        UINT64 GetBeginTime()
        {
            return mBeginTime;
        }

        // Whether timer is active.
        bool IsActive()
        {
            return mActive;
        }

    private:
        ID3D12Device* mpDevice;
        ID3D12QueryHeap* mQueryHeap;
        ID3D12Resource* mQueryResource;
        bool mActive;
        UINT64 mDeltaTime;
        UINT64 mBeginTime;
        UINT64 mEndTime;
        unsigned int mQueryCount;
};
