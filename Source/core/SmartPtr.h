#pragma once


template<typename ObjectType>
class RefCounter
{
public:
	RefCounter(ObjectType* obj)
			: m_Ptr(obj)
			, m_RefCount(0)
	{
	
	}

	~RefCounter()
	{
		ASSERT(m_RefCount == 0);
		m_Ptr = nullptr;
	}

	void AddRef() 
	{ 
		++m_RefCount; 
	}

	void DecRef()
	{
		ASSERT(m_RefCount);
		--m_RefCount;
	}

	u64 GetRefCount() const { return m_RefCount; }
	ObjectType* GetPtr() const { return m_Ptr; }

private:
	ObjectType* m_Ptr;
	std::atomic<u64> m_RefCount;
};

template<typename ObjectType>
class SharedPtr final
{
public:

	SharedPtr()
		: m_Ptr(nullptr)
		, m_Counter(nullptr)
	{
	}

	SharedPtr(ObjectType* obj)
		: m_Ptr(obj)
		, m_Counter(new RefCounter<ObjectType>(obj))
	{
		m_Counter->AddRef();
	}

	SharedPtr(RefCounter<ObjectType>& counter)
		: m_Counter(&counter)
		, m_Ptr(counter.GetPtr())
	{
		m_Counter->AddRef();
	}

	SharedPtr(SharedPtr<ObjectType> const& rhs)
		: m_Ptr(rhs.m_Ptr)
		, m_Counter(rhs.m_Counter)
	{
		m_Counter->AddRef();
	}

	SharedPtr<ObjectType> const& operator=(SharedPtr<ObjectType> const& rhs)
	{
		m_Ptr = rhs.m_Ptr;
		m_Counter = rhs.m_Counter;
		m_Counter->AddRef();
		return *this;
	}

	~SharedPtr()
	{
		if(IsValid())
		{
			m_Counter->DecRef();
			if (m_Counter->GetRefCount() == 0)
			{
				delete m_Counter;
				m_Counter = nullptr;

				delete m_Ptr;
				m_Ptr = nullptr;
			}
		}
	}
	bool IsValid() const { return m_Ptr; }

	ObjectType* operator->() { return m_Ptr; }

	ObjectType const* operator->() const { return m_Ptr; }

	ObjectType* Get() { return get(); }
	ObjectType const* Get() const { return get(); }

	// Compatibility with C++
	ObjectType* get() { return m_Ptr; }
	ObjectType const* get() const { return m_Ptr; }

	ObjectType& operator*()
	{
		ASSERT(m_Ptr);
		return *m_Ptr;
	}
	ObjectType const& operator*() const 
	{
		ASSERT(m_Ptr);
		return *m_Ptr;
	}


private:
	ObjectType* m_Ptr;
	RefCounter<ObjectType>* m_Counter;

};

template<typename ObjectType>
class WeakPtr
{
public:
	WeakPtr(SharedPtr<ObjectType> const& data)
		: m_Counter(data)
	{
	
	}

	~WeakPtr()
	{
	
	}

	SharedPtr<ObjectType> lock() 
	{
		if(m_Counter->GetRefCount() > 0)
		{
			return SharedPtr(m_Counter->m_Ptr);
		}
	}

private:
	ObjectType* m_Ptr;
	RefCounter<ObjectType>* m_Counter;

};