// Copyright (c) 2006-2008 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// See net/disk_cache/disk_cache.h for the public interface of the cache.

#ifndef NET_DISK_CACHE_BACKEND_IMPL_H_
#define NET_DISK_CACHE_BACKEND_IMPL_H_

#include "cache_block_files.h"
#include "disk_cache.h"
#include "cache_eviction.h"
#include "cache_rankings.h"
//#include "cache_trace.h"
#include "PlatformString.h"

#include <wtf/HashMap.h>

namespace disk_cache {

enum BackendFlags {
    kNone = 0,
    kMask = 1,                    // A mask (for the index table) was specified.
    kMaxSize = 1 << 1,            // A maximum size was provided.
    kUnitTestMode = 1 << 2,       // We are modifying the behavior for testing.
    kUpgradeMode = 1 << 3,        // This is the upgrade tool (dump).
    kNewEviction = 1 << 4,        // Use of new eviction was specified.
    kNoRandom = 1 << 5,           // Don't add randomness to the behavior.
    kNoLoadProtection = 1 << 6    // Don't act conservatively under load.
};

// This class implements the Backend interface. An object of this
// class handles the operations of the cache for a particular profile.
class BackendImpl : public Backend {
    friend class Eviction;
public:
    explicit BackendImpl(const FilePath& path)
        : path_(path), block_files_(path), mask_(0), max_size_(0),
            cache_type_(net::DISK_CACHE), uma_report_(0), user_flags_(0),
            init_(false), restarted_(false), read_only_(false),
//          new_eviction_(false), first_timer_(true)
            new_eviction_(true)
            {}
//        ALLOW_THIS_IN_INITIALIZER_LIST(factory_(this)) {}
  // mask can be used to limit the usable size of the hash table, for testing.
    BackendImpl(const FilePath& path, uint32 mask)
        : path_(path), block_files_(path), mask_(mask), max_size_(0),
            cache_type_(net::DISK_CACHE), uma_report_(0), user_flags_(kMask),
            init_(false), restarted_(false), read_only_(false),
//          new_eviction_(false), first_timer_(true)
            new_eviction_(true)
            {}
//        ALLOW_THIS_IN_INITIALIZER_LIST(factory_(this)) {}
    ~BackendImpl();
//add by suyong
    static Backend* CreateBackend(const char *full_path, bool force,
                                int max_bytes, net::CacheType type,
                                BackendFlags flags);

  // Returns a new backend with the desired flags. See the declaration of
  // CreateCacheBackend().
    static Backend* CreateBackend(const FilePath& full_path, bool force,
                             int max_bytes, net::CacheType type,
                              BackendFlags flags);

  // Performs general initialization for this current instance of the cache.
    bool Init();

  // Backend interface.
    virtual int32 GetEntryCount() const;
    virtual bool OpenEntry(const WTF::String& key, Entry** entry);
    virtual bool CreateEntry(const WTF::String& key, Entry** entry);

    virtual bool DoomEntry(const WTF::String& key);
    virtual bool clearCache();
    virtual bool DoomAllEntries();
    virtual bool OpenNextEntry(void** iter, Entry** next_entry);
    virtual void EndEnumeration(void** iter);
//  virtual void GetStats(StatsItems* stats);

  // Sets the maximum size for the total amount of data stored by this instance.
    virtual bool SetMaxSize(int max_bytes);

  //
    virtual bool GetMaxSize(int* size) {
        if (size)
        {
            *size = max_size_;
            return true;
        }
        return false;
    }

    virtual unsigned int GetUsedSize()
    {
        return (unsigned int)(data_->header.num_bytes);
    }

  // Sets the cache type for this backend.
    void SetType(net::CacheType type);

  // Returns the full name for an external storage file.
    FilePath GetFileName(Addr address) const;

  // Returns the actual file used to store a given (non-external) address.
    MappedFile* File(Addr address);

  // Creates an external storage file.
    bool CreateExternalFile(Addr* address);

  // Creates a new storage block of size block_count.
    bool CreateBlock(FileType block_type, int block_count,
                   Addr* block_address);

  // Deletes a given storage block. deep set to true can be used to zero-fill
  // the related storage in addition of releasing the related block.
    void DeleteBlock(Addr block_address, bool deep);

  // Retrieves a pointer to the lru-related data.
    LruData* GetLruData();

  // Updates the ranking information for an entry.
    void UpdateRank(EntryImpl* entry, bool modified);

  // A node was recovered from a crash, it may not be on the index, so this
  // method checks it and takes the appropriate action.
    void RecoveredEntry(CacheRankingsBlock* rankings);

  // Permanently deletes an entry, but still keeps track of it.
    void InternalDoomEntry(EntryImpl* entry);

  // Removes all references to this entry.
    void RemoveEntry(EntryImpl* entry);

  // This method must be called whenever an entry is released for the last time.
  // |address| is the cache address of the entry.
    void CacheEntryDestroyed(Addr address);

  // If the data stored by the provided |rankings| points to an open entry,
  // returns a pointer to that entry, otherwise returns NULL. Note that this
  // method does NOT increase the ref counter for the entry.
    EntryImpl* GetOpenEntry(CacheRankingsBlock* rankings) const;

  // Returns the id being used on this run of the cache.
    int32 GetCurrentEntryId() const;

  // Returns the maximum size for a file to reside on the cache.
    int MaxFileSize() const;

  // A user data block is being created, extended or truncated.
    void ModifyStorageSize(int32 old_size, int32 new_size);

  // Logs requests that are denied due to being too big.
  //void TooMuchStorageRequested(int32 size);

  // Returns true if this instance seems to be under heavy load.
    bool IsLoaded() const;

  // Returns the full histogram name, for the given base |name| and experiment,
  // and the current cache type. The name will be "DiskCache.t.name_e" where n
  // is the cache type and e the provided |experiment|.
//  std::string HistogramName(const char* name, int experiment) const;

    net::CacheType cache_type() const {
        return cache_type_;
    }

  // Returns the group for this client, based on the current cache size.
    int GetSizeGroup() const;

  // Reports a critical error (and disables the cache).
    void CriticalError(int error);

  // Reports an uncommon, recoverable error.
    void ReportError(int error);
  // Sets internal parameters to enable upgrade mode (for internal tools).
    void SetUpgradeMode();

  // Sets the eviction algorithm to version 2.
    void SetNewEviction();

  // Sets an explicit set of BackendFlags.
    void SetFlags(uint32 flags);

  // Peforms a simple self-check, and returns the number of dirty items
  // or an error code (negative value).
    int SelfCheck();

  // Same bahavior as OpenNextEntry but walks the list from back to front.
    bool OpenPrevEntry(void** iter, Entry** prev_entry);

private:
    typedef WTF::HashMap<CacheAddr, EntryImpl*> EntriesMap;

  // Creates a new backing file for the cache index.
    bool CreateBackingStore(disk_cache::File* file);
    bool InitBackingStore(bool* file_created);
    void AdjustMaxCacheSize(int table_len);

  // Deletes the cache and starts again.
    void RestartCache();
    void PrepareForRestart();

  // Creates a new entry object and checks to see if it is dirty. Returns zero
  // on success, or a disk_cache error on failure.
    int NewEntry(Addr address, EntryImpl** entry, bool* dirty);

  // Returns a given entry from the cache. The entry to match is determined by
  // key and hash, and the returned entry may be the matched one or it's parent
  // on the list of entries with the same hash (or bucket).
//  EntryImpl* MatchEntry(const std::string& key, uint32 hash, bool find_parent);//changed by suyong
     EntryImpl* MatchEntry(const WTF::String& key, uint32 hash, bool find_parent);

  // Opens the next or previous entry on a cache iteration.
    bool OpenFollowingEntry(bool forward, void** iter, Entry** next_entry);

  // Opens the next or previous entry on a single list. If successfull,
  // |from_entry| will be updated to point to the new entry, otherwise it will
  // be set to NULL; in other words, it is used as an explicit iterator.
    bool OpenFollowingEntryFromList(bool forward, Rankings::List list,
                                  CacheRankingsBlock** from_entry,
                                  EntryImpl** next_entry);

  // Returns the entry that is pointed by |next|. If we are trimming the cache,
  // |to_evict| should be true so that we don't perform extra disk writes.
    EntryImpl* GetEnumeratedEntry(CacheRankingsBlock* next, bool to_evict);

  // Re-opens an entry that was previously deleted.
    bool ResurrectEntry(EntryImpl* deleted_entry, Entry** entry);

    void DestroyInvalidEntry(EntryImpl* entry);
    void DestroyInvalidEntryFromEnumeration(EntryImpl* entry);

  // Handles the used storage count.
    void AddStorageSize(int32 bytes);
    void SubstractStorageSize(int32 bytes);

  // Update the number of referenced cache entries.
    void IncreaseNumRefs();
    void DecreaseNumRefs();
    void IncreaseNumEntries();
    void DecreaseNumEntries();

  // Upgrades the index file to version 2.1.
    void UpgradeTo2_1();

  // Performs basic checks on the index file. Returns false on failure.
    bool CheckIndex();

  // Part of the selt test. Returns the number or dirty entries, or an error.
    int CheckAllEntries();

  // Part of the self test. Returns false if the entry is corrupt.
    bool CheckEntry(EntryImpl* cache_entry);

    scoped_refptr<MappedFile> index_;  // The main cache index.
    FilePath path_;  // Path to the folder used as backing storage.
    Index* data_;  // Pointer to the index data.
    BlockFiles block_files_;  // Set of files used to store all data.
    Rankings rankings_;  // Rankings to be able to trim the cache.
    uint32 mask_;  // Binary mask to map a hash to the hash table.
    int32 max_size_;  // Maximum data size for this instance.
    Eviction eviction_;  // Handler of the eviction algorithm.
    EntriesMap open_entries_;  // Map of open entries.
    int num_refs_;  // Number of referenced cache entries.
    int max_refs_;  // Max number of referenced cache entries.
//  int num_pending_io_;  // Number of pending IO operations.
    net::CacheType cache_type_;
    int uma_report_;  // Controls transmision of UMA data.
    uint32 user_flags_;  // Flags set by the user.
    bool init_;  // controls the initialization of the system.
    bool restarted_;
//  bool unit_test_;
    bool read_only_;  // Prevents updates of the rankings data (used by tools).
    bool disabled_;
    bool new_eviction_;  // What eviction algorithm should be used.
//  bool first_timer_;  // True if the timer has not been called.

  //Stats stats_;  // Usage statistcs.
//  base::RepeatingTimer<BackendImpl> timer_;  // Usage timer.
  //scoped_refptr<TraceObject> trace_object_;  // Inits internal tracing.
//  ScopedRunnableMethodFactory<BackendImpl> factory_;

//  DISALLOW_EVIL_CONSTRUCTORS(BackendImpl);
};

// Returns the prefered max cache size given the available disk space.
    int PreferedCacheSize(int64 available);

}  // namespace disk_cache

#endif  // NET_DISK_CACHE_BACKEND_IMPL_H_
