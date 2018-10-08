/*
 * Copyright (C) 2018 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/**
 * @addtogroup NdkBinder
 * @{
 */

/**
 * @file binder_auto_utils.h
 * @brief These objects provide a more C++-like thin interface to the .
 */

#pragma once

#include <android/binder_ibinder.h>
#include <android/binder_parcel.h>
#include <android/binder_status.h>

#ifdef __cplusplus

#include <cstddef>

namespace ndk {

/**
 * Represents one strong pointer to an AIBinder object.
 */
class SpAIBinder {
public:
    /**
     * Takes ownership of one strong refcount of binder.
     */
    explicit SpAIBinder(AIBinder* binder = nullptr) : mBinder(binder) {}

    /**
     * Convenience operator for implicitly constructing an SpAIBinder from nullptr. This is not
     * explicit because it is not taking ownership of anything.
     */
    SpAIBinder(std::nullptr_t) : SpAIBinder() {}

    /**
     * This will delete the underlying object if it exists. See operator=.
     */
    SpAIBinder(const SpAIBinder& other) { *this = other; }

    /**
     * This deletes the underlying object if it exists. See set.
     */
    ~SpAIBinder() { set(nullptr); }

    /**
     * This takes ownership of a binder from another AIBinder object but it does not affect the
     * ownership of that other object.
     */
    SpAIBinder& operator=(const SpAIBinder& other) {
        AIBinder_incStrong(other.mBinder);
        set(other.mBinder);
        return *this;
    }

    /**
     * Takes ownership of one strong refcount of binder
     */
    void set(AIBinder* binder) {
        if (mBinder != nullptr) AIBinder_decStrong(mBinder);
        mBinder = binder;
    }

    /**
     * This returns the underlying binder object for transactions. If it is used to create another
     * SpAIBinder object, it should first be incremented.
     */
    AIBinder* get() const { return mBinder; }

    /**
     * This allows the value in this class to be set from beneath it. If you call this method and
     * then change the value of T*, you must take ownership of the value you are replacing and add
     * ownership to the object that is put in here.
     *
     * Recommended use is like this:
     *   SpAIBinder a;  // will be nullptr
     *   SomeInitFunction(a.getR());  // value is initialized with refcount
     *
     * Other usecases are discouraged.
     *
     */
    AIBinder** getR() { return &mBinder; }

private:
    AIBinder* mBinder = nullptr;
};

/**
 * This baseclass owns a single object, used to make various classes RAII.
 */
template <typename T, void (*Destroy)(T*)>
class ScopedA {
public:
    /**
     * Takes ownership of t.
     */
    explicit ScopedA(T* t = nullptr) : mT(t) {}

    /**
     * This deletes the underlying object if it exists. See set.
     */
    ~ScopedA() { set(nullptr); }

    /**
     * Takes ownership of t.
     */
    void set(T* t) {
        Destroy(mT);
        mT = t;
    }

    /**
     * This returns the underlying object to be modified but does not affect ownership.
     */
    T* get() { return mT; }

    /**
     * This returns the const underlying object but does not affect ownership.
     */
    const T* get() const { return mT; }

    /**
     * This allows the value in this class to be set from beneath it. If you call this method and
     * then change the value of T*, you must take ownership of the value you are replacing and add
     * ownership to the object that is put in here.
     *
     * Recommended use is like this:
     *   ScopedA<T> a; // will be nullptr
     *   SomeInitFunction(a.getR()); // value is initialized with refcount
     *
     * Other usecases are discouraged.
     *
     */
    T** getR() { return &mT; }

    // copy-constructing, or move/copy assignment is disallowed
    ScopedA(const ScopedA&) = delete;
    ScopedA& operator=(const ScopedA&) = delete;
    ScopedA& operator=(ScopedA&&) = delete;

    // move-constructing is okay
    ScopedA(ScopedA&&) = default;

private:
    T* mT;
};

/**
 * Convenience wrapper. See AParcel.
 */
class ScopedAParcel : public ScopedA<AParcel, AParcel_delete> {
public:
    /**
     * Takes ownership of a.
     */
    explicit ScopedAParcel(AParcel* a = nullptr) : ScopedA(a) {}
    ~ScopedAParcel() {}
    ScopedAParcel(ScopedAParcel&&) = default;
};

/**
 * Convenience wrapper. See AStatus.
 */
class ScopedAStatus : public ScopedA<AStatus, AStatus_delete> {
public:
    /**
     * Takes ownership of a.
     */
    explicit ScopedAStatus(AStatus* a = nullptr) : ScopedA(a) {}
    ~ScopedAStatus() {}
    ScopedAStatus(ScopedAStatus&&) = default;

    /**
     * See AStatus_isOk.
     */
    bool isOk() { return get() != nullptr && AStatus_isOk(get()); }
};

/**
 * Convenience wrapper. See AIBinder_DeathRecipient.
 */
class ScopedAIBinder_DeathRecipient
      : public ScopedA<AIBinder_DeathRecipient, AIBinder_DeathRecipient_delete> {
public:
    /**
     * Takes ownership of a.
     */
    explicit ScopedAIBinder_DeathRecipient(AIBinder_DeathRecipient* a = nullptr) : ScopedA(a) {}
    ~ScopedAIBinder_DeathRecipient() {}
    ScopedAIBinder_DeathRecipient(ScopedAIBinder_DeathRecipient&&) = default;
};

/**
 * Convenience wrapper. See AIBinder_Weak.
 */
class ScopedAIBinder_Weak : public ScopedA<AIBinder_Weak, AIBinder_Weak_delete> {
public:
    /**
     * Takes ownership of a.
     */
    explicit ScopedAIBinder_Weak(AIBinder_Weak* a = nullptr) : ScopedA(a) {}
    ~ScopedAIBinder_Weak() {}
    ScopedAIBinder_Weak(ScopedAIBinder_Weak&&) = default;

    /**
     * See AIBinder_Weak_promote.
     */
    SpAIBinder promote() { return SpAIBinder(AIBinder_Weak_promote(get())); }
};

} // namespace ndk

#endif // __cplusplus

/** @} */
