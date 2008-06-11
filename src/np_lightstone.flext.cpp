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
#include <lightstone.h>

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
 
public:
	// constructor
	np_lightstone() :
	mLightstone(NULL)
	{
		AddInAnything("Command Input (Bang for Update)");
		AddOutBang("Bangs on successful connection/command");
		AddOutFloat("Heart Rate");
		AddOutFloat("Skin Conductance");
		FLEXT_ADDMETHOD(0, lightstone_anything);
	} 

	virtual ~np_lightstone()
	{
		if(mLightstone)
		{
			close();
		}
	}
	
protected:
	lightstone mLightstone;
	lightstone_info mLightstoneInfo;

	void lightstone_anything(const t_symbol *msg,int argc,t_atom *argv)
	{				
		if(!strcmp(msg->s_name, "open"))
		{
			int ret;
			if(mLightstone)
			{
				lightstone_close(mLightstone);
			}
			if(argc == 1)
			{
				post("Opening %d", GetInt(argv[0]));
				ret = lightstone_open(&mLightstone, GetInt(argv[0]));
			}
			else
			{
				post("Opening default");
				ret = lightstone_open(&mLightstone, 0);
			}
			if(ret >= 0)
			{
				ToOutBang(0);
			}
			else
			{
				post("Cannot connect to lightstone");
			}
		}
		else if (!strcmp(msg->s_name, "count"))
		{
			post("lightstones Connected to System: %d", lightstone_get_count());
			ToOutBang(0);
		}
		else if (!strcmp(msg->s_name, "close"))
		{
			close();
			ToOutBang(0);
		}
		else if (!strcmp(msg->s_name, "bang"))
		{
			if(mLightstone)
			{
				mLightstoneInfo = lightstone_get_info(mLightstone);
				ToOutBang(0);
				ToOutFloat(1, mLightstoneInfo.hrv);
				ToOutFloat(2, mLightstoneInfo.scl);
			}
			else
			{
				post("Not connected to lightstone");
			}
		}
		else 
		{
			post("Not a valid np_lightstone message: %s", msg->s_name);
		}
	}

	void close()
	{
		if(mLightstone)
		{
			lightstone_close(mLightstone);
			mLightstone = NULL;
		}
	}

private:
	FLEXT_CALLBACK_A(lightstone_anything)
};

FLEXT_NEW("np_lightstone", np_lightstone)



