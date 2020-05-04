锘?// illusion.cpp : Defines the class behaviors for the application.
//



#include "stdafx.h"
#include "illusion_excel_file.h"
#include "illusion_read_config.h"
#include "illusion_main.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

Illusion_Application::Illusion_Application()
{

}


Illusion_Application::~Illusion_Application()
{

}


/// app镄勫紑濮嬭繍琛?int Illusion_Application::app_start(int /*argc*/, const char * /*argv*/[])
{
    int ret = 0;
    //浣跨敤UTF8浠ｇ爜涔熻緭鍑猴紝娴嬭瘯鍙戠幇杩樻槸链夊緢澶氶棶棰桡紝锲犱负镊