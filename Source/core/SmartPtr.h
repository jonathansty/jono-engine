#pragma once


template<typename ObjectType>
class RefCounter
{
public:
	RefCounter(ObjectType* obj)
	{
	
	}

	~RefCounter()
	{
	
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
class SharedPtr
{
public:
	SharedPtr(ObjectType* obj)
	{
		m_Counter = new RefCounter<ObjectType>(obj);
		m_Counter->AddRef();
	}

	SharedPtr(RefCounter<ObjectType>* counter)
	{
		m_Counter = counter;
		m_Ptr = counter->GetPtr();
		m_Counter->AddRef();
	}

	SharedPtr(SharedPtr<ObjectType> const& rhs)
	{

		m_Ptr = rhs->m_Ptr;
		m_Counter = rhs->m_Counter;
		m_Counter->AddRef();
	}

	SharedPtr<ObjectType> const& operator=(SharedPtr<ObjectType> const& rhs)
	{
		m_Ptr = rhs->m_Ptr;
		m_Counter = rhs->m_Counter;
		m_Counter->AddRef();
		return *this;
	}

	~SharedPtr()
	{
		m_Counter->DecRef();
		if(m_Counter->GetRefCount() == 0)
		{
			delete m_Counter;
			m_Counter = nullptr;

			delete m_Ptr;
			m_Ptr = nullptr;
		}
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
	{
	
	}

	~WeakPtr()
	{
	
	}

	SharedPtr<ObjectType> lock() 
	{
		if(m_Counter->GetRefCount() > 0)
		{
			return SharedPtr(m_Ptr);
		
		}
	}

private:
	RefCounter<ObjectType>* m_Counter;

};