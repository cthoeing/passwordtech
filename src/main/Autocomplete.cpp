// Autocomplete.cpp
//
// PASSWORD TECH
// Copyright (c) 2002-2023 by Christian Thoeing <c.thoeing@web.de>
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
// 02111-1307, USA.
//---------------------------------------------------------------------------
#include <shldisp.h>
#include <shlguid.h>
#include <algorithm>
#pragma hdrstop

#include "Autocomplete.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)

class AutoCompleteData : public IEnumString
{
public:
	AutoCompleteData(const std::vector<SecureWString>& items) :
		m_numRef(1), m_pos(0), m_items(items)
	{
	}

	STDMETHODIMP_(ULONG) AddRef()
	{
      return static_cast<ULONG>(InterlockedIncrement(&m_numRef));
	};

	STDMETHODIMP_(ULONG) Release()
	{
      const LONG r = InterlockedDecrement(&m_numRef);
      if (r == 0)
        delete this;
      return static_cast<ULONG>(r);
	};

	STDMETHODIMP QueryInterface(REFIID riid, void** ppv)
	{
      if (ppv == nullptr)
        return E_INVALIDARG;

      if(riid == IID_IUnknown)
      {
        IUnknown* p = this;
        *ppv = p;
        AddRef();
        return S_OK;
      }
      if(riid == IID_IEnumString)
      {
        IEnumString* p = this;
        *ppv = p;
        AddRef();
        return S_OK;
      }

      // The auto-completion object may ask for the optional IACList
      *ppv = nullptr;
      return E_NOINTERFACE;
	};

	STDMETHODIMP Next(ULONG celt, LPOLESTR* rgelt, ULONG* pceltFetched)
	{
      if (pceltFetched != nullptr)
        *pceltFetched = 0;

      if (celt == 0)
        return S_OK;
      if (rgelt == nullptr)
        return E_INVALIDARG;
      if (celt >= 2 && pceltFetched == nullptr)
        return E_INVALIDARG;

      const ULONG numAvailable = m_pos < m_items.size() ?
        m_items.size() - m_pos : 0;
      const ULONG numRet = std::min(celt, numAvailable);

      for (ULONG i = 0; i < numRet; ++i)
      {
        const auto& item = m_items[m_pos];
        const size_t len = item.StrLen();

        if (len == 0)
          continue;

        const size_t numBytes = (len + 1) * sizeof(WCHAR);

        LPVOID lpNew = CoTaskMemAlloc(numBytes);
        if(lpNew == nullptr)
          return E_OUTOFMEMORY;
        memcpy(lpNew, item.c_str(), numBytes);

        // Caller is responsible for freeing it
        rgelt[i] = reinterpret_cast<LPOLESTR>(lpNew);

        ++m_pos;
      }

      if (pceltFetched != nullptr)
        *pceltFetched = numRet;

      return (numRet == celt) ? S_OK : S_FALSE;
	};

	STDMETHODIMP Skip(ULONG celt)
	{
      const size_t newPos = m_pos + celt;
      if (newPos <= m_items.size() && newPos >= celt)
      {
        m_pos = newPos;
        return S_OK;
      }

      m_pos = m_items.size();
      return S_FALSE;
	};

	STDMETHODIMP Reset()
	{
      m_pos = 0;
      return S_OK;
	};

	STDMETHODIMP Clone(IEnumString** ppenum)
	{
      if (ppenum == nullptr)
        return E_INVALIDARG;

      AutoCompleteData* p = new AutoCompleteData(*this);

      *ppenum = p;
      return S_OK;
	};

private:
  AutoCompleteData(const AutoCompleteData& src)
    : m_numRef(1), m_items(src.m_items), m_pos(src.m_pos)
  {};

  LONG m_numRef;
  std::vector<SecureWString> m_items;
  size_t m_pos;
};

bool InitAutoComplete(HWND hWnd, const std::vector<SecureWString>& items)
{
  if (hWnd == nullptr || items.empty())
    return false;

  //HRESULT r=CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
  IAutoComplete2* pAC = nullptr;
  if (CoCreateInstance(CLSID_AutoComplete, nullptr, CLSCTX_INPROC_SERVER,
      IID_PPV_ARGS(&pAC)) != S_OK)
    return false;

  if (pAC == nullptr)
    return false;

  IUnknown* pEnum = new AutoCompleteData(items);

  HRESULT res = pAC->Init(hWnd, pEnum, nullptr, nullptr);
  if (res == S_OK)
    pAC->SetOptions(ACO_AUTOSUGGEST);

  pAC->Release();
  pEnum->Release();
  return res == S_OK;
};
