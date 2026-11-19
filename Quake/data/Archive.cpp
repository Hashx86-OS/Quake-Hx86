/*
 * MIT License
 *
 * Copyright (c) 2025 Malaka Gunawardana
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "Archive.h"

#include "HttpClient.h"
#include "StoreUtil.h"

#include <Hx86/Hsyscalls/syscalls.h>
#include <Hx86/debug.h>

#include <mbedtls/sha256.h>
#include <stdio.h>

#define MINIZ_NO_STDIO
#define MINIZ_NO_DEFLATE_APIS
#define MINIZ_NO_TIME
#define NDEBUG
#include <miniz.h>

extern "C" {
#include "lha_input_stream.h"
#include "lha_reader.h"
}

namespace {

constexpr const char* kZipUrl = "https://www.gamers.org/pub/idgames/idstuff/quake/quake106.zip";
constexpr uint32_t kZipSize = 9094045;
constexpr const char* kZipHash = "ec6c9d34b1ae0252ac0066045b6611a7919c2a0d78a3a66d9387a8f597553239";

constexpr uint32_t kResourceSize = 9086574;
constexpr const char* kResourceHash =
    "c192c9c71bee41750dd7d14c99378766d61e077977b9d13d1a457b8d9eabe34a";
constexpr const char* kResourceMember = "resource.1";

constexpr uint32_t kPakSize = 18689235;
constexpr const char* kPakHash = "35a9c55e5e5a284a159ad2a62e0e8def23d829561fe2f54eb402dbc0a9a946af";
constexpr uint32_t kSlicnseSize = 10036;
constexpr uint32_t kLicinfoSize = 9311;
constexpr const char* kSlicnseHash =
    "070cdf6a6410adef8fb5f83a4e5ccdb9e2301d2e48d460bb3a67a0f5ba9d70a8";
constexpr const char* kLicinfoHash =
    "8d654834d085c088d603fb6879a874799e8b4cc6ae9317bbf826afd83bc72e31";

constexpr uint32_t kPathCapacity = 320;

/** struct ArchivePaths - Paths rooted at the directory containing Quake.bin. */
struct ArchivePaths {
    char sharewareDir[kPathCapacity];
    char licensesDir[kPathCapacity];
    char dataDir[kPathCapacity];
    char zipPath[kPathCapacity];
    char zipPartPath[kPathCapacity];
    char resourcePath[kPathCapacity];
    char resourcePartPath[kPathCapacity];
    char pakPath[kPathCapacity];
    char pakPartPath[kPathCapacity];
    char slicnsePath[kPathCapacity];
    char slicnsePartPath[kPathCapacity];
    char licinfoPath[kPathCapacity];
    char licinfoPartPath[kPathCapacity];
};

/**
 * JoinPath() - Join a base directory and a relative leaf, avoiding double
 * slashes and tolerating a trailing slash on the base.
 */
void JoinPath(char* output, uint32_t capacity, const char* base, const char* leaf) {
    uint32_t used = 0;
    if (base != nullptr) {
        while (*base != '\0' && used + 1 < capacity) output[used++] = *base++;
    }
    while (used > 0 && output[used - 1] == '/') used--;
    if (used + 1 < capacity) output[used++] = '/';
    if (leaf != nullptr) {
        while (*leaf != '\0' && used + 1 < capacity) output[used++] = *leaf++;
    }
    output[used] = '\0';
}

void BuildPaths(const char* installDir, ArchivePaths* paths) {
    JoinPath(paths->sharewareDir, kPathCapacity, installDir, "shareware");
    JoinPath(paths->licensesDir, kPathCapacity, installDir, "licenses");
    JoinPath(paths->dataDir, kPathCapacity, installDir, "id1");
    JoinPath(paths->zipPath, kPathCapacity, installDir, "shareware/quake106.zip");
    JoinPath(paths->zipPartPath, kPathCapacity, installDir, "shareware/quake106.zip.part");
    JoinPath(paths->resourcePath, kPathCapacity, installDir, "shareware/resource.1");
    JoinPath(paths->resourcePartPath, kPathCapacity, installDir, "shareware/resource.1.part");
    JoinPath(paths->pakPath, kPathCapacity, installDir, "id1/pak0.pak");
    JoinPath(paths->pakPartPath, kPathCapacity, installDir, "id1/pak0.pak.part");
    JoinPath(paths->slicnsePath, kPathCapacity, installDir, "licenses/SLICNSE.TXT");
    JoinPath(paths->slicnsePartPath, kPathCapacity, installDir, "licenses/SLICNSE.TXT.part");
    JoinPath(paths->licinfoPath, kPathCapacity, installDir, "licenses/LICINFO.TXT");
    JoinPath(paths->licinfoPartPath, kPathCapacity, installDir, "licenses/LICINFO.TXT.part");
}

struct ShaSink {
    int32_t fd;
    mbedtls_sha256_context sha;
    uint32_t received;
    uint32_t expected;
    uint32_t nextProgress;
    ArchiveProgress progress;
    const char* label;
};

void SetError(char* error, uint32_t capacity, const char* text) {
    StoreStrCopy(error, capacity, text);
}

void Report(ArchiveProgress progress, const char* text) {
    if (progress != nullptr) progress(text);
}

void DigestToHex(const uint8_t digest[32], char output[65]) {
    static const char kHex[] = "0123456789abcdef";
    for (uint32_t i = 0; i < 32; i++) {
        output[i * 2] = kHex[digest[i] >> 4];
        output[i * 2 + 1] = kHex[digest[i] & 0x0F];
    }
    output[64] = '\0';
}

bool FinishSha(ShaSink* sink, char output[65]) {
    uint8_t digest[32] = {};
    int result = mbedtls_sha256_finish_ret(&sink->sha, digest);
    mbedtls_sha256_free(&sink->sha);
    if (result != 0) return false;
    DigestToHex(digest, output);
    return true;
}

void UpdateProgress(ShaSink* sink, const char* label) {
    if (sink->progress == nullptr) return;
    if (sink->expected == 0 || sink->received < sink->nextProgress) return;
    uint32_t percent = (sink->received * 100u) / sink->expected;
    char text[160];
    StoreFormat(text, sizeof(text), "%s: %u / %u KB (%u%%)", label, sink->received / 1024u,
                sink->expected / 1024u, percent);
    sink->progress(text);
    sink->nextProgress += 64u * 1024u;
}

bool DownloadSink(void* context, const uint8_t* data, uint32_t length) {
    ShaSink* sink = static_cast<ShaSink*>(context);
    if (!StoreWriteAll(sink->fd, data, length)) return false;
    if (mbedtls_sha256_update_ret(&sink->sha, data, length) != 0) return false;
    sink->received += length;
    UpdateProgress(sink, sink->label);
    return true;
}

bool ReplaceFile(const char* partPath, const char* finalPath) {
    syscall_unlink(finalPath);
    return syscall_rename(partPath, finalPath) == 0;
}

bool DownloadFile(const char* url, const char* partPath, const char* finalPath,
                  uint32_t expectedSize, const char* expectedHash, const char* label,
                  ArchiveProgress progress, char* error, uint32_t errorCapacity) {
    int32_t fd = syscall_open(partPath, O_WRONLY | O_CREAT | O_TRUNC);
    if (fd < 0) {
        StoreFormat(error, errorCapacity, "could not create %s", partPath);
        return false;
    }

    ShaSink sink = {};
    sink.fd = fd;
    sink.expected = expectedSize;
    sink.nextProgress = 64u * 1024u;
    sink.progress = progress;
    sink.label = label;
    mbedtls_sha256_init(&sink.sha);
    if (mbedtls_sha256_starts_ret(&sink.sha, 0) != 0) {
        syscall_close((uint32_t)fd);
        syscall_unlink(partPath);
        mbedtls_sha256_free(&sink.sha);
        SetError(error, errorCapacity, "SHA-256 initialization failed");
        return false;
    }

    StoreHttpResponse response = {};
    bool ok = StoreHttpGet(url, "*/*", DownloadSink, &sink, &response, error, errorCapacity);
    char digest[65];
    bool hashOk = FinishSha(&sink, digest);
    syscall_close((uint32_t)fd);

    if (!ok) {
        syscall_unlink(partPath);
        return false;
    }
    if (!hashOk || sink.received != expectedSize || !StoreHexEqual(digest, expectedHash)) {
        syscall_unlink(partPath);
        SetError(error, errorCapacity, "downloaded file failed its size or SHA-256 check");
        return false;
    }
    if (!ReplaceFile(partPath, finalPath)) {
        syscall_unlink(partPath);
        StoreFormat(error, errorCapacity, "could not move %s into place", finalPath);
        return false;
    }
    return true;
}

struct ZipSource {
    int32_t fd;
    uint32_t size;
};

size_t ZipRead(void* opaque, mz_uint64 offset, void* buffer, size_t length) {
    ZipSource* source = static_cast<ZipSource*>(opaque);
    if (offset >= source->size) return 0;
    uint32_t available = source->size - (uint32_t)offset;
    if (length > available) length = available;
    if (syscall_lseek((uint32_t)source->fd, (int32_t)offset, SEEK_SET) < 0) return 0;
    int32_t result = syscall_read((uint32_t)source->fd, (char*)buffer, (uint32_t)length);
    return result > 0 ? (size_t)result : 0;
}

size_t ZipWrite(void* opaque, mz_uint64 offset, const void* data, size_t length) {
    (void)offset;
    ShaSink* sink = static_cast<ShaSink*>(opaque);
    if (!StoreWriteAll(sink->fd, (const uint8_t*)data, (uint32_t)length)) return 0;
    if (mbedtls_sha256_update_ret(&sink->sha, (const uint8_t*)data, length) != 0) return 0;
    sink->received += (uint32_t)length;
    UpdateProgress(sink, sink->label);
    return length;
}

bool ExtractResourceFromZip(const ArchivePaths& paths, ArchiveProgress progress, char* error,
                            uint32_t errorCapacity) {
    uint32_t zipSize = 0;
    if (!StoreFileExists(paths.zipPath, &zipSize) || zipSize != kZipSize) {
        SetError(error, errorCapacity, "shareware ZIP is missing or has the wrong size");
        return false;
    }

    int32_t zipFd = syscall_open(paths.zipPath, O_RDONLY);
    if (zipFd < 0) {
        SetError(error, errorCapacity, "could not open the shareware ZIP");
        return false;
    }

    ZipSource source = {zipFd, zipSize};
    mz_zip_archive zip = {};
    zip.m_pRead = ZipRead;
    zip.m_pIO_opaque = &source;
    if (!mz_zip_reader_init(&zip, zipSize, 0)) {
        syscall_close((uint32_t)zipFd);
        SetError(error, errorCapacity, "shareware ZIP is not readable");
        return false;
    }

    int index = mz_zip_reader_locate_file(&zip, kResourceMember, nullptr, 0);
    mz_zip_archive_file_stat stat = {};
    if (index < 0 || !mz_zip_reader_file_stat(&zip, (mz_uint)index, &stat) ||
        stat.m_uncomp_size != kResourceSize) {
        mz_zip_reader_end(&zip);
        syscall_close((uint32_t)zipFd);
        SetError(error, errorCapacity, "resource.1 is missing from the shareware ZIP");
        return false;
    }

    int32_t outFd = syscall_open(paths.resourcePartPath, O_WRONLY | O_CREAT | O_TRUNC);
    if (outFd < 0) {
        mz_zip_reader_end(&zip);
        syscall_close((uint32_t)zipFd);
        SetError(error, errorCapacity, "could not create the resource.1 temporary file");
        return false;
    }

    ShaSink sink = {};
    sink.fd = outFd;
    sink.expected = kResourceSize;
    sink.nextProgress = 64u * 1024u;
    sink.progress = progress;
    sink.label = "Extracting shareware archive";
    mbedtls_sha256_init(&sink.sha);
    bool started = mbedtls_sha256_starts_ret(&sink.sha, 0) == 0;
    bool extracted =
        started && mz_zip_reader_extract_to_callback(&zip, (mz_uint)index, ZipWrite, &sink, 0);
    char digest[65];
    bool hashOk = FinishSha(&sink, digest);
    syscall_close((uint32_t)outFd);
    mz_zip_reader_end(&zip);
    syscall_close((uint32_t)zipFd);

    if (!started || !extracted || !hashOk || sink.received != kResourceSize ||
        !StoreHexEqual(digest, kResourceHash)) {
        syscall_unlink(paths.resourcePartPath);
        SetError(error, errorCapacity, "resource.1 failed its size or SHA-256 check");
        return false;
    }
    if (!ReplaceFile(paths.resourcePartPath, paths.resourcePath)) {
        syscall_unlink(paths.resourcePartPath);
        SetError(error, errorCapacity, "could not store resource.1");
        return false;
    }
    return true;
}

struct LhaFile {
    int32_t fd;
};

int LhaRead(void* handle, void* buffer, size_t length) {
    LhaFile* file = static_cast<LhaFile*>(handle);
    int32_t result = syscall_read((uint32_t)file->fd, (char*)buffer, (uint32_t)length);
    return result < 0 ? -1 : result;
}

void LhaClose(void* handle) {
    LhaFile* file = static_cast<LhaFile*>(handle);
    if (file->fd >= 0) syscall_close((uint32_t)file->fd);
    file->fd = -1;
}

int LhaSkip(void* handle, size_t bytes) {
    LhaFile* file = static_cast<LhaFile*>(handle);
    if (bytes > 0x7FFFFFFFu) return 0;
    return syscall_lseek((uint32_t)file->fd, (int32_t)bytes, SEEK_CUR) >= 0;
}

struct LhaTarget {
    const char* name;
    const char* partPath;
    const char* finalPath;
    uint32_t size;
    const char* hash;
    const char* label;
    bool found;
};

bool ExtractLhaTarget(LHAReader* reader, LhaTarget* target, ArchiveProgress progress, char* error,
                      uint32_t errorCapacity) {
    int32_t fd = syscall_open(target->partPath, O_WRONLY | O_CREAT | O_TRUNC);
    if (fd < 0) {
        StoreFormat(error, errorCapacity, "could not create %s", target->partPath);
        return false;
    }

    ShaSink sink = {};
    sink.fd = fd;
    sink.expected = target->size;
    sink.nextProgress = 64u * 1024u;
    sink.progress = progress;
    sink.label = target->label;
    mbedtls_sha256_init(&sink.sha);
    if (mbedtls_sha256_starts_ret(&sink.sha, 0) != 0) {
        syscall_close((uint32_t)fd);
        syscall_unlink(target->partPath);
        mbedtls_sha256_free(&sink.sha);
        SetError(error, errorCapacity, "SHA-256 initialization failed");
        return false;
    }

    uint8_t buffer[16384];
    size_t count;
    while ((count = lha_reader_read(reader, buffer, sizeof(buffer))) > 0) {
        if (!StoreWriteAll(fd, buffer, (uint32_t)count) ||
            mbedtls_sha256_update_ret(&sink.sha, buffer, count) != 0) {
            syscall_close((uint32_t)fd);
            syscall_unlink(target->partPath);
            mbedtls_sha256_free(&sink.sha);
            SetError(error, errorCapacity, "failed while writing the extracted data");
            return false;
        }
        sink.received += (uint32_t)count;
        UpdateProgress(&sink, target->label);
    }

    char digest[65];
    bool hashOk = FinishSha(&sink, digest);
    syscall_close((uint32_t)fd);
    if (!hashOk || sink.received != target->size || !StoreHexEqual(digest, target->hash)) {
        syscall_unlink(target->partPath);
        StoreFormat(error, errorCapacity, "%s failed its size or SHA-256 check", target->name);
        return false;
    }
    if (!ReplaceFile(target->partPath, target->finalPath)) {
        syscall_unlink(target->partPath);
        StoreFormat(error, errorCapacity, "could not move %s into place", target->finalPath);
        return false;
    }
    target->found = true;
    return true;
}

bool ExtractSharewareMembers(const ArchivePaths& paths, ArchiveProgress progress, char* error,
                             uint32_t errorCapacity) {
    int32_t fd = syscall_open(paths.resourcePath, O_RDONLY);
    if (fd < 0) {
        SetError(error, errorCapacity, "could not open resource.1");
        return false;
    }

    LhaFile file = {fd};
    LHAInputStreamType type = {LhaRead, LhaSkip, LhaClose};
    LHAInputStream* stream = lha_input_stream_new(&type, &file);
    if (stream == nullptr) {
        syscall_close((uint32_t)fd);
        SetError(error, errorCapacity, "could not create the LHA input stream");
        return false;
    }
    LHAReader* reader = lha_reader_new(stream);
    if (reader == nullptr) {
        lha_input_stream_free(stream);
        SetError(error, errorCapacity, "could not open the LHA archive");
        return false;
    }

    LhaTarget targets[] = {
        {"id1/pak0.pak", paths.pakPartPath, paths.pakPath, kPakSize, kPakHash,
         "Extracting pak0.pak", false},
        {"slicnse.txt", paths.slicnsePartPath, paths.slicnsePath, kSlicnseSize, kSlicnseHash,
         "Extracting license", false},
        {"licinfo.txt", paths.licinfoPartPath, paths.licinfoPath, kLicinfoSize, kLicinfoHash,
         "Extracting license", false},
    };

    bool failed = false;
    for (;;) {
        LHAFileHeader* header = lha_reader_next_file(reader);
        if (header == nullptr) break;

        char fullName[192];
        fullName[0] = '\0';
        if (header->path != nullptr) StoreStrCopy(fullName, sizeof(fullName), header->path);
        if (header->filename != nullptr) {
            uint32_t used = StoreStrLen(fullName);
            StoreStrCopy(fullName + used, sizeof(fullName) - used, header->filename);
        }

        for (uint32_t i = 0; i < sizeof(targets) / sizeof(targets[0]); i++) {
            if (!targets[i].found && StoreStrCaseEqual(fullName, targets[i].name)) {
                if (!ExtractLhaTarget(reader, &targets[i], progress, error, errorCapacity)) {
                    failed = true;
                }
                break;
            }
        }
        if (failed) break;
    }

    lha_reader_free(reader);
    lha_input_stream_free(stream);

    if (failed) return false;
    for (uint32_t i = 0; i < sizeof(targets) / sizeof(targets[0]); i++) {
        if (!targets[i].found) {
            StoreFormat(error, errorCapacity, "%s is missing from the shareware archive",
                        targets[i].name);
            return false;
        }
    }
    return true;
}

}  // namespace

bool ArchiveInstallSharewareData(const char* installDir, ArchiveProgress progress, char* error,
                                 uint32_t errorCapacity) {
    if (error != nullptr && errorCapacity != 0) error[0] = '\0';

    ArchivePaths paths = {};
    BuildPaths(installDir, &paths);

    if (StoreFileExists(paths.pakPath, nullptr)) {
        Report(progress, "Game data is already installed.");
        return true;
    }

    if (!StoreEnsureDir(paths.sharewareDir) || !StoreEnsureDir(paths.licensesDir) ||
        !StoreEnsureDir(paths.dataDir)) {
        SetError(error, errorCapacity, "could not create the Quake data directories");
        return false;
    }

    Report(progress, "Downloading the shareware archive...");
    if (!DownloadFile(kZipUrl, paths.zipPartPath, paths.zipPath, kZipSize, kZipHash,
                      "Downloading shareware archive", progress, error, errorCapacity)) {
        return false;
    }

    Report(progress, "Extracting resource.1...");
    if (!ExtractResourceFromZip(paths, progress, error, errorCapacity)) return false;

    Report(progress, "Extracting game data...");
    if (!ExtractSharewareMembers(paths, progress, error, errorCapacity)) return false;

    syscall_unlink(paths.resourcePath);
    Report(progress, "Shareware episode 1 data installed.");
    return true;
}
