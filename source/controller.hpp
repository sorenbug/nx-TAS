#pragma once
#include <switch.h>
#include <cstring>
#include <string>
#include "script_provider.hpp"
#include "script_init.hpp"

extern Event vsync_event;

class TasController
{
    private:

    u64 HdlsHandle = 0;
    HiddbgHdlsDeviceInfo device = {0};
    HiddbgHdlsState state = {0};
    static char controllerCount;

    public:

    TasController(int controllerType, uint8_t bodyR, uint8_t bodyG, uint8_t bodyB, uint8_t buttonR, uint8_t buttonG, uint8_t buttonB);
    TasController();
    ~TasController();
    void pressA();
    void pressLR();

    void waitForVsync();
    void setInputNextFrame();

    template<class T, class... Args> void runScript(Args&&... args)
    {
        auto provider = std::make_shared<T>(std::forward<Args>(args)...);
        if(!provider->isGood()) return;

        provider->populateQueue();
        std::shared_ptr<struct controlMsg> nextLine = provider->nextLine();
        provider->populateQueue();

        int currentFrame = 0;

        while(provider->hasNextLine() || nextLine->frame >= currentFrame)
        {
            if(nextLine->frame == currentFrame)
            {
                runMsg(nextLine);
                if(provider->hasNextLine())
                {
                    nextLine.reset();
                    nextLine = provider->nextLine();
                }
            }
            else
            {
                emptyMsg();
            }

            pushProvider(provider);

            setInputNextFrame();

            currentFrame++;

        }
        emptyMsg();
        setInputNextFrame();

        nextLine.reset();

        waitForVsync();
    }

    void runMsg(std::shared_ptr<struct controlMsg> msg);
    void emptyMsg();
};
