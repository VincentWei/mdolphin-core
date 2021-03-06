// Copyright (c) 2006-2009 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#include "string_util.h"
#include "backend_impl.h"
#include "cache_bitmap.h"
#include "cache_util.h"
#include "cache_entry_impl.h"
#include "CString.h"
#include "PlatformString.h"
#include "mdolphin_debug.h"

using base::Time;
using base::TimeDelta;

namespace {

// Index for the file used to store the key, if any (files_[kKeyFileIndex]).
const int kKeyFileIndex = 3;

// Clears buffer before offset and after valid_len, knowing that the size of
// buffer is kMaxBlockSize.
void ClearInvalidData(char* buffer, int offset, int valid_len)
{
    //  DCHECK(offset >= 0);
    //  DCHECK(valid_len >= 0);
    //  DCHECK(disk_cache::kMaxBlockSize >= offset + valid_len);
    if (offset)
        memset(buffer, 0, offset);
    int end = disk_cache::kMaxBlockSize - offset - valid_len;
    if (end)
        memset(buffer + offset + valid_len, 0, end);
}

}  // namespace

namespace disk_cache {

EntryImpl::EntryImpl(BackendImpl* backend, Addr address)
    : entry_(NULL, Addr(0)), node_(NULL, Addr(0))
{
        entry_.LazyInit(backend->File(address), address);
        doomed_ = false;
        backend_ = backend;
        for (int i = 0; i < kNumStreams; i++) {
            unreported_size_[i] = 0;
        }
        key_file_ = NULL;
}

// When an entry is deleted from the cache, we clean up all the data associated
// with it for two reasons: to simplify the reuse of the block (we know that any
// unused block is filled with zeros), and to simplify the handling of write /
// read partial information from an entry (don't have to worry about returning
// data related to a previous cache entry because the range was not fully
// written before).
EntryImpl::~EntryImpl()
{
    // md_debug("******** ~EntryImpl [%s]*************\n", GetKey().utf8().data());
    // Save the sparse info to disk before deleting this entry.
    //  sparse_.reset();
    if (doomed_) {
        // md_debug("********* delete entry Data ***********\n");
        DeleteEntryData(true);
    } else {
        bool ret = true;
        for (int index = 0; index < kNumStreams; index++) {
            if (user_buffers_[index].get()) {
                if (!(ret = Flush(index, entry_.Data()->data_size[index], false)))
                {
                    // md_debug("flush error\n");
                } else
                {}
                    // md_debug("flush ok\n");
                //          LOG(ERROR) << "Failed to save user data";
                //          ;
            } else if (unreported_size_[index]) {
                backend_->ModifyStorageSize(
                        entry_.Data()->data_size[index] - unreported_size_[index],
                        entry_.Data()->data_size[index]);
            }
        }

        if (!ret) {
            // There was a failure writing the actual data. Mark the entry as dirty.
            int current_id = backend_->GetCurrentEntryId();
            node_.Data()->dirty = current_id == 1 ? -1 : current_id - 1;
            node_.Store();
        } else if (node_.HasData() && node_.Data()->dirty) {
            node_.Data()->dirty = 0;
            node_.Store();
        }
    }

    backend_->CacheEntryDestroyed(entry_.address());
}

void EntryImpl::Doom()
{
    if (doomed_)
        return;

    SetPointerForInvalidEntry(backend_->GetCurrentEntryId());
    backend_->InternalDoomEntry(this);
}

void EntryImpl::Close()
{
    Release();
}

//std::string EntryImpl::GetKey() const {
WTF::String EntryImpl::GetKey() const
{	
    CacheEntryBlock* entry = const_cast<CacheEntryBlock*>(&entry_);
    if (entry->Data()->key_len <= kMaxInternalKeyLength)
        //    return std::string(entry->Data()->key);
        return WTF::String(entry->Data()->key);

    Addr address(entry->Data()->long_key);
    //  DCHECK(address.is_initialized());
    size_t offset = 0;
    if (address.is_block_file())
        offset = address.start_block() * address.BlockSize() + kBlockHeaderSize;

    if (!key_file_) {
        // We keep a copy of the file needed to access the key so that we can
        // always return this object's key, even if the backend is disabled.
        COMPILE_ASSERT(kNumStreams == kKeyFileIndex, invalid_key_index);
        key_file_ = const_cast<EntryImpl*>(this)->GetBackingFile(address,
                kKeyFileIndex);
    }

    //  std::string key;newUninitialized
    /*
    UChar *buf; //add by suyong
    key.newUninitialized(size_t(entry->Data()->key_len + 1), buf); //add by suyong
    */
    String key1;
    char *buf = new char[entry->Data()->key_len];
    if (!key_file_ ||
        !key_file_->Read(buf, entry->Data()->key_len, offset)){
        delete []buf;
        return key1;
    }
    //WTF::String key(buf, entry->Data()->key_len); 
    WTF::String key = WTF::String::fromUTF8(buf, entry->Data()->key_len);

    delete []buf;
    return key;
}

Time EntryImpl::GetLastUsed() const
 {
    CacheRankingsBlock* node = const_cast<CacheRankingsBlock*>(&node_);
    return Time::FromInternalValue(node->Data()->last_used);
}

Time EntryImpl::GetLastModified() const
{
    CacheRankingsBlock* node = const_cast<CacheRankingsBlock*>(&node_);
    return Time::FromInternalValue(node->Data()->last_modified);
}

unsigned EntryImpl::GetDataSize(int index) const
{
    if (index < 0 || index >= kNumStreams)
        return 0;

    CacheEntryBlock* entry = const_cast<CacheEntryBlock*>(&entry_);
    return entry->Data()->data_size[index];
}

int EntryImpl::ReadData(int index, int offset, char* buf, int buf_len,
        net::CompletionCallback* completion_callback)
{
    //int EntryImpl::ReadData(int index, int offset, char* buf, int buf_len)
    //  DCHECK(node_.Data()->dirty);
    if (index < 0 || index >= kNumStreams)
        //    return net::ERR_INVALID_ARGUMENT;
        return -1;

    int entry_size = entry_.Data()->data_size[index];
    if (offset >= entry_size || offset < 0 || !buf_len)
        return 0;

    if (buf_len < 0)
        return -1;
    //    return net::ERR_INVALID_ARGUMENT;

    Time start = Time::Now();

    if (offset + buf_len > entry_size)
        buf_len = entry_size - offset;

    UpdateRank(false);

    //  backend_->OnEvent(Stats::READ_DATA);

    if (user_buffers_[index].get()) {
        // Complete the operation locally.
        //    DCHECK(kMaxBlockSize >= offset + buf_len);
        //    memcpy(buf->data() , user_buffers_[index].get() + offset, buf_len);
        memcpy(buf , user_buffers_[index].get() + offset, buf_len);
        //    ReportIOTime(kRead, start);
        return buf_len;
    }

    Addr address(entry_.Data()->data_addr[index]);
    //  DCHECK(address.is_initialized());
    if (!address.is_initialized())
        return -2;
    //    return net::ERR_FAILED;

    File* file = GetBackingFile(address, index);
    if (!file)
        return -2;
    //    return net::ERR_FAILED;

    size_t file_offset = offset;
    if (address.is_block_file())
        file_offset += address.start_block() * address.BlockSize() +
            kBlockHeaderSize;

    //  SyncCallback* io_callback = NULL;
    //  if (completion_callback)
    //    io_callback = new SyncCallback(this, buf, completion_callback);

    bool completed = true;
    //  if (!file->Read(buf->data(), buf_len, file_offset, NULL, &completed))
    if (!file->Read(buf, buf_len, file_offset)) {
        //    if (io_callback)
        //      io_callback->Discard();
        return -2;
        //    return net::ERR_FAILED;
    }

    //  if (io_callback && completed)
    //    io_callback->Discard();

    //  ReportIOTime(kRead, start);
    //  return (completed || !completion_callback) ? buf_len : net::ERR_IO_PENDING;
    return completed ? buf_len : -1;
}

int EntryImpl::WriteData(int index, int offset, const char* buf, int buf_len,
        net::CompletionCallback* completion_callback,
        bool truncate)
{
    //int EntryImpl::WriteData(int index, int offset, const char* buf, int buf_len,
    //                         bool truncate) {
    //  DCHECK(node_.Data()->dirty);
    if (index < 0 || index >= kNumStreams)
        return -1;
    //    return net::ERR_INVALID_ARGUMENT;

    if (offset < 0 || buf_len < 0)
        return -1;
    //    return net::ERR_INVALID_ARGUMENT;

    int max_file_size = backend_->MaxFileSize();

    // offset of buf_len could be negative numbers.
    if (offset > max_file_size || buf_len > max_file_size ||
            offset + buf_len > max_file_size) {
        int size = offset + buf_len;
        if (size <= max_file_size)
            size = kint32max;
        //backend_->TooMuchStorageRequested(size);
        //    return net::ERR_FAILED;
        // md_debug("error failed\n");
        return -2;
    }

    Time start = Time::Now();

    // Read the size at this point (it may change inside prepare).
    int entry_size = entry_.Data()->data_size[index];
    if (!PrepareTarget(index, offset, buf_len, truncate))
        return -2;
    //    return net::ERR_FAILED;

    if (entry_size < offset + buf_len) {
        unreported_size_[index] += offset + buf_len - entry_size;
        entry_.Data()->data_size[index] = offset + buf_len;
        entry_.set_modified();
        if (!buf_len)
            truncate = true;  // Force file extension.
    } else if (truncate) {
        // If the size was modified inside PrepareTarget, we should not do
        // anything here.
        if ((entry_size > offset + buf_len) &&
                (entry_size == entry_.Data()->data_size[index])) {
            unreported_size_[index] += offset + buf_len - entry_size;
            entry_.Data()->data_size[index] = offset + buf_len;
            entry_.set_modified();
        } else {
            // Nothing to truncate.
            truncate = false;
        }
    }

    UpdateRank(true);

    //  backend_->OnEvent(Stats::WRITE_DATA);

    if (user_buffers_[index].get()) {
        // Complete the operation locally.
        if (!buf_len)
            return 0;

        //    DCHECK(kMaxBlockSize >= offset + buf_len);
        //    memcpy(user_buffers_[index].get() + offset, buf->data(), buf_len);
        memcpy(user_buffers_[index].get() + offset, buf, buf_len);
        //    ReportIOTime(kWrite, start);
        return buf_len;
    }

    Addr address(entry_.Data()->data_addr[index]);
    File* file = GetBackingFile(address, index);
    if (!file)
    {
        // md_debug("GetBackingFile failed\n");
        return -2;
    }
    //    return net::ERR_FAILED;

    size_t file_offset = offset;
    if (address.is_block_file()) {
        file_offset += address.start_block() * address.BlockSize() +
            kBlockHeaderSize;
    } else if (truncate) {
        if (!file->SetLength(offset + buf_len))
            return -2;
        //      return net::ERR_FAILED;
    }

    if (!buf_len)
        return 0;

    //  SyncCallback* io_callback = NULL;
    //  if (completion_callback)
    //    io_callback = new SyncCallback(this, buf, completion_callback);

    bool completed = true;
    if (!file->Write(buf, buf_len, file_offset)) {
        //  if (!file->Write(buf->data(), buf_len, file_offset, io_callback,
        //                   &completed)) {
        //    if (io_callback)
        //      io_callback->Discard();
        return -2;
        //    return net::ERR_FAILED;
    }

    //  if (io_callback && completed)
    //    io_callback->Discard();

    //  ReportIOTime(kWrite, start);
    //  return (completed || !completion_callback) ? buf_len : net::ERR_IO_PENDING;
    return completed  ? buf_len : -1;
}

int EntryImpl::GetAvailableRange(int64 offset, int len, int64* start) 
{
    /*  int result = InitSparseData();
        if (net::OK != result)
        return result;

        return sparse_->GetAvailableRange(offset, len, start);
    */
    return 0;
}
    // ------------------------------------------------------------------------

uint32 EntryImpl::GetHash() 
{
    return entry_.Data()->hash;
}

//bool EntryImpl::CreateEntry(Addr node_address, const std::string& key,
bool EntryImpl::CreateEntry(Addr node_address, const WTF::String& key,	
        uint32 hash) 
{
    //Trace("Create entry In");
    EntryStore* entry_store = entry_.Data();
    RankingsNode* node = node_.Data();
    memset(entry_store, 0, sizeof(EntryStore) * entry_.address().num_blocks());
    memset(node, 0, sizeof(RankingsNode));
    if (!node_.LazyInit(backend_->File(node_address), node_address))
        return false;

    entry_store->rankings_node = node_address.value();
    node->contents = entry_.address().value();

    entry_store->hash = hash;
    entry_store->creation_time = Time::Now().ToInternalValue();
    //  entry_store->key_len = static_cast<int32>(key.size());
    entry_store->key_len = static_cast<int32>(key.utf8().length());  
    if (entry_store->key_len > kMaxInternalKeyLength) {
        Addr address(0);
        if (!CreateBlock(entry_store->key_len + 1, &address))
            return false;

        entry_store->long_key = address.value();
        key_file_ = GetBackingFile(address, kKeyFileIndex);

        size_t offset = 0;
        if (address.is_block_file())
            offset = address.start_block() * address.BlockSize() + kBlockHeaderSize;

        //    if (!key_file_ || !key_file_->Write(key.data(), key.size(), offset)) {
        if (!key_file_ || !key_file_->Write(key.utf8().data(), key.utf8().length(), offset)) {		
            DeleteData(address, kKeyFileIndex);
            return false;
        }

        if (address.is_separate_file())
            //      key_file_->SetLength(key.size() + 1);
            key_file_->SetLength(key.utf8().length() + 1);	
    } else {
        //    memcpy(entry_store->key, key.data(), key.size());
        memcpy(entry_store->key, key.utf8().data(), key.utf8().length());	
        //    entry_store->key[key.size()] = '\0';
        entry_store->key[key.utf8().length()] = '\0';	
    }
    //  backend_->ModifyStorageSize(0, static_cast<int32>(key.size()));
    backend_->ModifyStorageSize(0, static_cast<int32>(key.utf8().length()));
    node->dirty = backend_->GetCurrentEntryId();
    //  Log("Create Entry ");
    return true;
}

//bool EntryImpl::IsSameEntry(const std::string& key, uint32 hash) {//changed by suyong
bool EntryImpl::IsSameEntry(const WTF::String& key, uint32 hash)
{
    if (entry_.Data()->hash != hash ||
            //      static_cast<size_t>(entry_.Data()->key_len) != key.size())
        unsigned (entry_.Data()->key_len) != key.utf8().length())
            return false;

    //  std::string my_key = GetKey();
    WTF::String my_key = GetKey();  
    //  return key.compare(my_key) ? false : true;
    return key==my_key? true : false;
}

void EntryImpl::InternalDoom() 
{
    //  DCHECK(node_.HasData());
    if (!node_.Data()->dirty) {
        node_.Data()->dirty = backend_->GetCurrentEntryId();
        node_.Store();
    }
    doomed_ = true;
}

void EntryImpl::DeleteEntryData(bool everything) 
{
    //  DCHECK(doomed_ || !everything);
    for (int index = 0; index < kNumStreams; index++) {
        Addr address(entry_.Data()->data_addr[index]);
        if (address.is_initialized()) {
            DeleteData(address, index);
            backend_->ModifyStorageSize(entry_.Data()->data_size[index] -
                    unreported_size_[index], 0);
            // md_debug("****DELETE size %d\n", entry_.Data()->data_size[index] - unreported_size_[index]);
            entry_.Data()->data_addr[index] = 0;
            entry_.Data()->data_size[index] = 0;
        }
    }

    if (!everything) {
        entry_.Store();
        return;
    }

    // Remove all traces of this entry.
    backend_->RemoveEntry(this);

    Addr address(entry_.Data()->long_key);
    DeleteData(address, kKeyFileIndex);
    backend_->ModifyStorageSize(entry_.Data()->key_len, 0);

    memset(node_.buffer(), 0, node_.size());
    memset(entry_.buffer(), 0, entry_.size());
    node_.Store();
    entry_.Store();

    backend_->DeleteBlock(node_.address(), false);
    backend_->DeleteBlock(entry_.address(), false);
}

CacheAddr EntryImpl::GetNextAddress() 
{
    return entry_.Data()->next;
}

void EntryImpl::SetNextAddress(Addr address) 
{
    entry_.Data()->next = address.value();
    //  bool success = entry_.Store();
    //  DCHECK(success);
}

bool EntryImpl::LoadNodeAddress() 
{
    Addr address(entry_.Data()->rankings_node);
    if (!node_.LazyInit(backend_->File(address), address))
        return false;
    return node_.Load();
}

bool EntryImpl::Update() 
{
    //  DCHECK(node_.HasData());

    RankingsNode* rankings = node_.Data();
    if (!rankings->dirty) {
        rankings->dirty = backend_->GetCurrentEntryId();
        if (!node_.Store())
            return false;
    }
    return true;
}

bool EntryImpl::IsDirty(int32 current_id) 
{
    //  DCHECK(node_.HasData());
    // We are checking if the entry is valid or not. If there is a pointer here,
    // we should not be checking the entry.
    if (node_.Data()->dummy)
        return true;

    return node_.Data()->dirty && current_id != node_.Data()->dirty;
}

void EntryImpl::ClearDirtyFlag() 
{
    node_.Data()->dirty = 0;
}

void EntryImpl::SetPointerForInvalidEntry(int32 new_id) 
{
    node_.Data()->dirty = new_id;
    node_.Data()->dummy = 0;
    node_.Store();
}

bool EntryImpl::SanityCheck() 
{
        if (!entry_.Data()->rankings_node || !entry_.Data()->key_len)
            return false;

        Addr rankings_addr(entry_.Data()->rankings_node);
        if (!rankings_addr.is_initialized() || rankings_addr.is_separate_file() ||
                rankings_addr.file_type() != RANKINGS)
            return false;

        Addr next_addr(entry_.Data()->next);
        if (next_addr.is_initialized() &&
                (next_addr.is_separate_file() || next_addr.file_type() != BLOCK_256))
            return false;

        return true;
}

//void EntryImpl::IncrementIoCount() {
//  backend_->IncrementIoCount();
//}

//void EntryImpl::DecrementIoCount() {
//  backend_->DecrementIoCount();
//}

void EntryImpl::SetTimes(base::Time last_used, base::Time last_modified) {
    node_.Data()->last_used = last_used.ToInternalValue();
    node_.Data()->last_modified = last_modified.ToInternalValue();
    node_.set_modified();
}
// ------------------------------------------------------------------------

bool EntryImpl::CreateDataBlock(int index, int size) {
    //  DCHECK(index >= 0 && index < kNumStreams);

    Addr address(entry_.Data()->data_addr[index]);
    if (!CreateBlock(size, &address))
        return false;

    entry_.Data()->data_addr[index] = address.value();
    entry_.Store();
    return true;
}

bool EntryImpl::CreateBlock(int size, Addr* address) {
    //  DCHECK(!address->is_initialized());

    FileType file_type = Addr::RequiredFileType(size);
    if (EXTERNAL == file_type) {
        if (size > backend_->MaxFileSize())
            return false;
        if (!backend_->CreateExternalFile(address))
            return false;
    } else {
        int num_blocks = (size + Addr::BlockSizeForFileType(file_type) - 1) /
            Addr::BlockSizeForFileType(file_type);

        if (!backend_->CreateBlock(file_type, num_blocks, address))
            return false;
    }
    return true;
}

void EntryImpl::DeleteData(Addr address, int index) 
{
    if (!address.is_initialized())
        return;
    if (address.is_separate_file()) {
        if (files_[index])
            files_[index] = NULL;  // Releases the object.

        int failure = DeleteCacheFile(backend_->GetFileName(address)) ? 0 : 1;
        //    CACHE_UMA(COUNTS, "DeleteFailed", 0, failure);
        if (failure)
        {
            // md_debug("********** delete file [%s] failed*************\n", 
               //     backend_->GetFileName(address).value().utf8().data());
        }
        //      LOG(ERROR) << "Failed to delete " <<
        //         backend_->GetFileName(address).value() << " from the cache.";
    } else {
        backend_->DeleteBlock(address, true);
    }
}

void EntryImpl::UpdateRank(bool modified) 
{
    if (!doomed_) {
        // Everything is handled by the backend.
        backend_->UpdateRank(this, true);
        return;
    }

    Time current = Time::Now();
    node_.Data()->last_used = current.ToInternalValue();

    if (modified)
        node_.Data()->last_modified = current.ToInternalValue();
}

File* EntryImpl::GetBackingFile(Addr address, int index) 
{
    File* file;
    if (address.is_separate_file())
        file = GetExternalFile(address, index);
    else
        file = backend_->File(address);
    return file;
}

File* EntryImpl::GetExternalFile(Addr address, int index) 
{
    //  DCHECK(index >= 0 && index <= kKeyFileIndex);
    if (!files_[index].get()) {
        // For a key file, use mixed mode IO.
        scoped_refptr<File> file(new File(kKeyFileIndex == index));
        if (file->Init(backend_->GetFileName(address)))
            files_[index].swap(file);
    }
    return files_[index].get();
}

bool EntryImpl::PrepareTarget(int index, int offset, int buf_len,
        bool truncate) 
{
    Addr address(entry_.Data()->data_addr[index]);

    if (address.is_initialized() || user_buffers_[index].get())
        return GrowUserBuffer(index, offset, buf_len, truncate);

    if (offset + buf_len > kMaxBlockSize)
        return CreateDataBlock(index, offset + buf_len);

    user_buffers_[index].reset(new char[kMaxBlockSize]);

    // Overwrite the parts of the buffer that are not going to be written
    // by the current operation (and yes, let's assume that nothing is going
    // to fail, and we'll actually write over the part that we are not cleaning
    // here). The point is to avoid writing random stuff to disk later on.
    ClearInvalidData(user_buffers_[index].get(), offset, buf_len);

    return true;
}

// We get to this function with some data already stored. If there is a
// truncation that results on data stored internally, we'll explicitly
// handle the case here.
bool EntryImpl::GrowUserBuffer(int index, int offset, int buf_len,
        bool truncate) 
{
    Addr address(entry_.Data()->data_addr[index]);

    if (offset + buf_len > kMaxBlockSize) {
        // The data has to be stored externally.
        if (address.is_initialized()) {
            if (address.is_separate_file())
                return true;
            if (!MoveToLocalBuffer(index))
                return false;
        }
        return Flush(index, offset + buf_len, true);
    }

    if (!address.is_initialized()) {
        //    DCHECK(user_buffers_[index].get());
        if (truncate)
            ClearInvalidData(user_buffers_[index].get(), 0, offset + buf_len);
        return true;
    }
    if (address.is_separate_file()) {
        if (!truncate)
            return true;
        //    return ImportSeparateFile(index, offset, buf_len);
    }

    // At this point we are dealing with data stored on disk, inside a block file.
    if (offset + buf_len <= address.BlockSize() * address.num_blocks())
        return true;

    // ... and the allocated block has to change.
    if (!MoveToLocalBuffer(index))
        return false;

    int clear_start = entry_.Data()->data_size[index];
    if (truncate)
        clear_start = std::min(clear_start, offset + buf_len);
    else if (offset < clear_start)
        clear_start = std::max(offset + buf_len, clear_start);

    // Clear the end of the buffer.
    ClearInvalidData(user_buffers_[index].get(), 0, clear_start);
    return true;
}

bool EntryImpl::MoveToLocalBuffer(int index) 
{
    Addr address(entry_.Data()->data_addr[index]);
    //  DCHECK(!user_buffers_[index].get());
    //  DCHECK(address.is_initialized());
    scoped_array<char> buffer(new char[kMaxBlockSize]);

    File* file = GetBackingFile(address, index);
    size_t len = entry_.Data()->data_size[index];
    size_t offset = 0;

    if (address.is_block_file())
        offset = address.start_block() * address.BlockSize() + kBlockHeaderSize;

    //  if (!file || !file->Read(buffer.get(), len, offset, NULL, NULL))
    if (!file || !file->Read(buffer.get(), len, offset))
        return false;

    DeleteData(address, index);
    entry_.Data()->data_addr[index] = 0;
    entry_.Store();

    // If we lose this entry we'll see it as zero sized.
    backend_->ModifyStorageSize(static_cast<int>(len) - unreported_size_[index],
            0);
    unreported_size_[index] = static_cast<int>(len);

    user_buffers_[index].swap(buffer);
    return true;
}
// The common scenario is that this is called from the destructor of the entry,
// to write to disk what we have buffered. We don't want to hold the destructor
// until the actual IO finishes, so we'll send an asynchronous write that will
// free up the memory containing the data. To be consistent, this method always
// returns with the buffer freed up (on success).
bool EntryImpl::Flush(int index, int size, bool async) 
{
    Addr address(entry_.Data()->data_addr[index]);
    //  DCHECK(user_buffers_[index].get());
    //  DCHECK(!address.is_initialized());

    if (!size)
        return true;

    if (!CreateDataBlock(index, size))
        return false;

    address.set_value(entry_.Data()->data_addr[index]);

    File* file = GetBackingFile(address, index);
    size_t len = entry_.Data()->data_size[index];
    size_t offset = 0;
    if (address.is_block_file())
        offset = address.start_block() * address.BlockSize() + kBlockHeaderSize;

    // We just told the backend to store len bytes for real.
    //  DCHECK(len == static_cast<size_t>(unreported_size_[index]));
    backend_->ModifyStorageSize(0, static_cast<int>(len));
    unreported_size_[index] = 0;

    if (!file)
        return false;

    // TODO(rvargas): figure out if it's worth to re-enable posting operations.
    // Right now it is only used from GrowUserBuffer, not the destructor, and
    // it is not accounted for from the point of view of the total number of
    // pending operations of the cache. It is also racing with the actual write
    // on the GrowUserBuffer path because there is no code to exclude the range
    // that is going to be written.

    async = false;
    if (!file->Write(user_buffers_[index].get(), len, offset)) //add by suyong
        return false;
    user_buffers_[index].reset(NULL);

    // The buffer is deleted from the PostWrite operation.
    user_buffers_[index].release();

    return true;
}

void EntryImpl::SetEntryFlags(uint32 flags) 
{
    entry_.Data()->flags |= flags;
    entry_.set_modified();
}

uint32 EntryImpl::GetEntryFlags() 
{
    return entry_.Data()->flags;
}

void EntryImpl::GetData(int index, char** buffer, Addr* address) {
    if (user_buffers_[index].get()) {
        // The data is already in memory, just copy it an we're done.
        int data_len = entry_.Data()->data_size[index];
        //    DCHECK(data_len <= kMaxBlockSize);
        *buffer = new char[data_len];
        memcpy(*buffer, user_buffers_[index].get(), data_len);
        return;
    }

    // Bad news: we'd have to read the info from disk so instead we'll just tell
    // the caller where to read from.
    *buffer = NULL;
    address->set_value(entry_.Data()->data_addr[index]);
    if (address->is_initialized()) {
        // Prevent us from deleting the block from the backing store.
        backend_->ModifyStorageSize(entry_.Data()->data_size[index] -
                unreported_size_[index], 0);
        entry_.Data()->data_addr[index] = 0;
        entry_.Data()->data_size[index] = 0;
    }
}

}  // namespace disk_cache
