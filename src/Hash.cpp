/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */
#include "Hash.h"
#include "Logging.h"
#include "Subprocess.h"
#ifdef _WIN32
#include <windows.h>
#include <wincrypt.h>
#endif

#include <sstream>

std::string
fileHash(std::string path)
{
#if !defined(_WIN32)
  std::ostringstream ss;
#if defined(__apple_build_version__)
  ss << "md5 -r " << path;
#else
  ss << "md5sum " << path;
#endif

  std::string hash = getCmdOutput(ss.str());
  // strip everything past actual hash string
  return hash.substr(0, hash.find(" "));
#else
  HANDLE file = CreateFileA(path.c_str(), GENERIC_READ, FILE_SHARE_READ,
                            nullptr, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN,
                            nullptr);
  if (file == INVALID_HANDLE_VALUE)
    return "";

  // Per http://msdn.microsoft.com/en-us/library/aa382380%28VS.85%29.aspx
  HCRYPTPROV provider = 0;
  HCRYPTHASH hash = 0;

  if(!CryptAcquireContext(&provider, NULL, NULL, PROV_RSA_FULL,
                          CRYPT_VERIFYCONTEXT))
  {
    CloseHandle(file);
    return "";
  }

  bool failed = false;
  if(!CryptCreateHash(provider, CALG_MD5, 0, 0, &hash))
  {
    failed = true;
  }

  BYTE bytes[1024];
  DWORD read = 0;
  while (!failed && ReadFile(file, bytes, sizeof(bytes), &read, nullptr))
  {
    if (read == 0)
      break;
    if (!CryptHashData(hash, bytes, read, 0))
    {
      failed = true;
    }
  }

  std::string result;
  if (failed)
    return agentWarn("Failed to hash file");

  BYTE hash_bytes[16];
  DWORD size = sizeof(hash_bytes);
  if (CryptGetHashParam(hash, HP_HASHVAL, hash_bytes, &size, 0))
  {
    for (DWORD i = 0; i < size; i++)
    {
      char buf[3];
      _snprintf_s(buf, 3, "%02x", hash_bytes[i]);
      result.append(buf);
    }
  }

  CloseHandle(file);
  if (provider)
    CryptReleaseContext(provider, 0);
  if (hash)
    CryptDestroyHash(hash);

  return result;
#endif
}
