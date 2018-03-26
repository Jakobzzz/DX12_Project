#pragma once

#include <d3d12.h>
#include <Windows.h>

// D3D12 timer.
class D3D12Timer
{
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
		m_framesPerSecond = 0;

		D3D12_QUERY_HEAP_DESC queryHeapDesc;
		queryHeapDesc.Type = D3D12_QUERY_HEAP_TYPE_TIMESTAMP;
		queryHeapDesc.NodeMask = 0;
		queryHeapDesc.Count = mQueryCount;

		QueryPerformanceFrequency(&m_qpcFrequency);
		QueryPerformanceCounter(&m_qpcLastTime);

		// Initialize max delta to 1/10 of a second.
		m_qpcMaxDelta = m_qpcFrequency.QuadPart / 10;

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

	// Get elapsed time since the previous Update call.
	UINT64 GetElapsedTicks() const { return m_elapsedTicks; }
	double GetElapsedSeconds() const { return TicksToSeconds(m_elapsedTicks); }

	// Get total time since the start of the program.
	UINT64 GetTotalTicks() const { return m_totalTicks; }
	double GetTotalSeconds() const { return TicksToSeconds(m_totalTicks); }

	// Get total number of updates since start of the program.
	UINT32 GetFrameCount() const { return m_frameCount; }

	// Set whether to use fixed or variable timestep mode.
	void SetFixedTimeStep(bool isFixedTimestep) { m_isFixedTimeStep = isFixedTimestep; }

	// Set how often to call Update when in fixed timestep mode.
	void SetTargetElapsedTicks(UINT64 targetElapsed) { m_targetElapsedTicks = targetElapsed; }
	void SetTargetElapsedSeconds(double targetElapsed) { m_targetElapsedTicks = SecondsToTicks(targetElapsed); }

	static UINT64 SecondsToTicks(double seconds) { return static_cast<UINT64>(seconds * TicksPerSecond); }

	// Get the current framerate.
	UINT32 GetFramesPerSecond() const { return m_framesPerSecond; }

	static double TicksToSeconds(UINT64 ticks) { return static_cast<double>(ticks) / TicksPerSecond; }

	typedef void(*LPUPDATEFUNC) (void);

	void ResetElapsedTime()
	{
		QueryPerformanceCounter(&m_qpcLastTime);

		m_leftOverTicks = 0;
		m_framesPerSecond = 0;
		m_framesThisSecond = 0;
		m_qpcSecondCounter = 0;
	}

	// Update timer state, calling the specified Update function the appropriate number of times.
	void Tick(LPUPDATEFUNC update)
	{
		// Query the current time.
		LARGE_INTEGER currentTime;

		QueryPerformanceCounter(&currentTime);

		UINT64 timeDelta = currentTime.QuadPart - m_qpcLastTime.QuadPart;

		m_qpcLastTime = currentTime;
		m_qpcSecondCounter += timeDelta;

		// Clamp excessively large time deltas (e.g. after paused in the debugger).
		if (timeDelta > m_qpcMaxDelta)
		{
			timeDelta = m_qpcMaxDelta;
		}

		// Convert QPC units into a canonical tick format. This cannot overflow due to the previous clamp.
		timeDelta *= TicksPerSecond;
		timeDelta /= m_qpcFrequency.QuadPart;

		UINT32 lastFrameCount = m_frameCount;

		if (m_isFixedTimeStep)
		{
			// Fixed timestep update logic

			// If the app is running very close to the target elapsed time (within 1/4 of a millisecond) just clamp
			// the clock to exactly match the target value. This prevents tiny and irrelevant errors
			// from accumulating over time. Without this clamping, a game that requested a 60 fps
			// fixed update, running with vsync enabled on a 59.94 NTSC display, would eventually
			// accumulate enough tiny errors that it would drop a frame. It is better to just round 
			// small deviations down to zero to leave things running smoothly.

			if (abs(static_cast<int>(timeDelta - m_targetElapsedTicks)) < TicksPerSecond / 4000)
			{
				timeDelta = m_targetElapsedTicks;
			}

			m_leftOverTicks += timeDelta;

			while (m_leftOverTicks >= m_targetElapsedTicks)
			{
				m_elapsedTicks = m_targetElapsedTicks;
				m_totalTicks += m_targetElapsedTicks;
				m_leftOverTicks -= m_targetElapsedTicks;
				m_frameCount++;

				if (update)
				{
					update();
				}
			}
		}
		else
		{
			// Variable timestep update logic.
			m_elapsedTicks = timeDelta;
			m_totalTicks += timeDelta;
			m_leftOverTicks = 0;
			m_frameCount++;

			if (update)
			{
				update();
			}
		}

		// Track the current framerate.
		if (m_frameCount != lastFrameCount)
		{
			m_framesThisSecond++;
		}

		if (m_qpcSecondCounter >= static_cast<UINT64>(m_qpcFrequency.QuadPart))
		{
			m_framesPerSecond = m_framesThisSecond;
			m_framesThisSecond = 0;
			m_qpcSecondCounter %= m_qpcFrequency.QuadPart;
		}
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
	ID3D12Device * mpDevice;
	ID3D12QueryHeap* mQueryHeap;
	ID3D12Resource* mQueryResource;
	bool mActive;
	UINT64 mDeltaTime;
	UINT64 mBeginTime;
	UINT64 mEndTime;

	//Source timing data uses QPC units.
	LARGE_INTEGER m_qpcFrequency;
	LARGE_INTEGER m_qpcLastTime;
	UINT64 m_qpcMaxDelta;
	unsigned int mQueryCount;

	//Derived timing data uses a canonical tick format.
	UINT64 m_elapsedTicks;
	UINT64 m_totalTicks;
	UINT64 m_leftOverTicks;

	//Members for tracking the framerate.
	UINT32 m_frameCount;
	UINT32 m_framesPerSecond;
	UINT32 m_framesThisSecond;
	UINT64 m_qpcSecondCounter;

	//Members for configuring fixed timestep mode.
	bool m_isFixedTimeStep;
	UINT64 m_targetElapsedTicks;

	// Integer format represents time using 10,000,000 ticks per second.
	static const UINT64 TicksPerSecond = 10000000;
};
