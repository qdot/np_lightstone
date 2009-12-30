/*
 * Implementation file for Trancevibrator Max/Pd External
 *
 * Copyright (c) 2005-2008 Kyle Machulis/Nonpolynomial Labs <kyle@nonpolynomial.com>
 *
 * More info on Nonpolynomial Labs @ http://www.nonpolynomial.com
 *
 * Sourceforge project @ http://www.sourceforge.net/projects/libtrancevibe/
 *
 * Example code from flext tutorials. http://www.parasitaere-kapazitaeten.net/ext/flext/
 */

// include flext header
#include <flext.h>
#include "lightstone/lightstone.h"

// check for appropriate flext version
#if !defined(FLEXT_VERSION) || (FLEXT_VERSION < 400)
#error You need at least flext version 0.4.0
#endif


class np_lightstone:
	// inherit from basic flext class
	public flext_base
{
	// obligatory flext header (class name,base class name)
	FLEXT_HEADER(np_lightstone,flext_base)

	// Same as boost ScopedMutex, just using flext's mutex class.
	class ScopedMutex
	{
		ScopedMutex() {}

	public:
		ScopedMutex(ThrMutex& tm)
		{
			m = &tm;
			m->Lock();
		}

		~ScopedMutex()
		{
			m->Unlock();
		}
	private:
		ThrMutex* m;
	};

public:
	// constructor
	np_lightstone() :
		m_lightstoneDevice(lightstone_create()),
		m_shouldRun(false)
	{
		AddInAnything("Command Input (Bang for Update)");
		AddOutBang("Bangs on successful connection/command");
		AddOutFloat("Heart Rate");
		AddOutFloat("Skin Conductance");
		FLEXT_ADDMETHOD_(0, "close", close);
		FLEXT_ADDMETHOD_(0, "start", start);
		FLEXT_ADDMETHOD_(0, "stop", stop);
		FLEXT_ADDMETHOD_(0, "count", count);
		FLEXT_ADDMETHOD(0, anything);

		post("Lightstone External v1.0.0");
		post("by Nonpolynomial Labs (http://www.nonpolynomial.com)");
		post("Updates at http://www.github.com/qdot/np_lightstone");
		post("Compiled on " __DATE__ " " __TIME__);
	} 

	virtual ~np_lightstone()
	{
		if(m_lightstoneDevice->_is_open)
		{
			close();
		}
		lightstone_delete(m_lightstoneDevice);
	}
	
protected:
	lightstone* m_lightstoneDevice;
	lightstone_info m_lightstoneDeviceInfo;
	bool m_shouldRun;
	
	void anything(const t_symbol *msg,int argc,t_atom *argv)
	{				
		if(!strcmp(msg->s_name, "open"))
		{
			int ret;
			if(m_lightstoneDevice->_is_open)
			{
				close();
			}
			if(argc == 1)
			{
				post("np_lightstone - Opening %d", GetInt(argv[0]));
				ret = lightstone_open(m_lightstoneDevice, GetInt(argv[0]));
			}
			else
			{
				post("np_lightstone - Opening first device");
				ret = lightstone_open(m_lightstoneDevice, 0);
			}
			if(ret >= 0)
			{
				ToOutBang(0);
			}
			else
			{
				post("np_lightstone - Cannot connect to lightstone");
			}
			return;
		}
		if (!strcmp(msg->s_name, "bang"))
		{
			poll();
			return;
		}
		post("np_lightstone - Not a valid np_lightstone message: %s", msg->s_name);
	}

	void count()
	{
		post("np_lightstone - lightstones Connected to System: %d", lightstone_get_count(m_lightstoneDevice));
		ToOutBang(0);
	}
	
	void close()
	{
		if(!m_lightstoneDevice->_is_open)
		{
			post("np_lightstone - No device currently open");
			return;
		}
		if(m_shouldRun)
		{
			stop();
		}
		lightstone_close(m_lightstoneDevice);
		post("np_lightstone - Device closed");
	}

	
	void stop()
	{
		if(!m_shouldRun)
		{
			post("np_lightstone - No I/O thread currently running");
			return;
		}
		m_shouldRun = false;
	}

	void start()
	{
		if(!m_lightstoneDevice->_is_open)
		{
			Lock();
			post("np_lightstone - No device currently open");
			Unlock();
			return;
		}
		m_shouldRun = true;
		while(m_shouldRun)
		{
			poll();
			//We can sleep for a while here. The lightstone doesn't move at anywhere near 100hz. More like 20-30hz.
			Sleep(.01);
		}
		Lock();
		post("np_lightstone - Exiting thread loop");
		Unlock();
	}

	void poll()
	{
		if(!m_lightstoneDevice->_is_open)
		{
			post("np_lightstone - No device currently open");
			return;
		}
		m_lightstoneDeviceInfo = lightstone_get_info(m_lightstoneDevice);
		Lock();
		ToOutBang(0);
		ToOutFloat(1, m_lightstoneDeviceInfo.hrv);
		ToOutFloat(2, m_lightstoneDeviceInfo.scl);
		Unlock();
	}
	
private:
	FLEXT_CALLBACK_A(anything)
	FLEXT_THREAD(start)
	FLEXT_CALLBACK(count)
	FLEXT_CALLBACK(close)
	FLEXT_CALLBACK(stop)
};

FLEXT_NEW("np_lightstone", np_lightstone)



