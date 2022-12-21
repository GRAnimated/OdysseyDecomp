#pragma once

#include <basis/seadTypes.h>
#include <heap/seadDisposer.h>

#include <al/Library/System/SystemKit.h>

#include "RootTask.h"

namespace al {
    class GameFrameworkNx;
    class AccountHolder;
}    // namespace al

class Application {
    SEAD_SINGLETON_DISPOSER(Application)
public:
    Application();
<<<<<<< HEAD
    void init(int, char **);
    void run();
    RootTask *getRootTask() const;

    al::SystemKit *mSystemKit;
    al::GameFrameworkNx *mFramework;
    u64 _30;
    al::AccountHolder *mAccountHolder;
=======
    void init(int, char**);
    void run();
    RootTask* getRootTask() const;

    al::SystemKit* mSystemKit;
    al::GameFrameworkNx* mFramework;
    u64 _30;
    al::AccountHolder* mAccountHolder;
>>>>>>> 60112915b1b559e06290092c73b9c070ba03786b
};