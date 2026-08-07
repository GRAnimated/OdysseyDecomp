#pragma once

#include <container/seadPtrArray.h>
#include <prim/seadSafeString.h>

namespace al {
class LiveActor;
}

class PlayerModelHolder {
public:
    friend class PlayerActorHakoniwa;

    struct Entry {
        Entry(al::LiveActor* actor) : actor(actor) {}

        sead::FixedSafeString<128> name;
        al::LiveActor* actor;
    };

    PlayerModelHolder(u32 bufferSize);
    void registerModel(al::LiveActor* actor, const char* name);
    void changeModel(const char* name);
    al::LiveActor* findModelActor(const char* name) const;
    al::LiveActor* tryFindModelActor(const char* name) const;
    bool isCurrentModelLabel(const char* name) const;
    bool isCurrentModelLabelSubString(const char* name) const;

    al::LiveActor* getCurrentModelActor() const { return mCurrentModel->actor; }

    void setModelSuffix(const char* suffix) { _18.format("%s", suffix); }

    const sead::SafeString& getModelSuffix() const { return _18; }

private:
    sead::PtrArray<Entry> mBuffer;
    Entry* mCurrentModel = nullptr;
    sead::FixedSafeString<128> _18 = sead::FixedSafeString<128>("");
};

static_assert(sizeof(PlayerModelHolder::Entry) == 0xA0);
static_assert(sizeof(PlayerModelHolder) == 0xB0);
