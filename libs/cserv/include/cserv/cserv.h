#pragma once

#define CS_TYPE_PROCESS 1


template<class T> class cspacket{
	public:
		cspacket(int type,int buffer_size);
		virtual ~cspacket();
		virtual T* get_buffer();
		virtual int get_buffer_size();
		virtual int get_type();
	private:
		int type;
		T* buffer;
		int buffer_size;
};

template<class T> void send_packet(cspacket<T>* packet);
