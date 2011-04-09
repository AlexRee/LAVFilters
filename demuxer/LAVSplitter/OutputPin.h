/*
 *      Copyright (C) 2011 Hendrik Leppkes
 *      http://www.1f0.de
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 *  http://www.gnu.org/copyleft/gpl.html
 *
 *  Initial design and concept by Gabest and the MPC-HC Team, copyright under GPLv2
 *  Contributions by Ti-BEN from the XBMC DSPlayer Project, also under GPLv2
 */

#pragma once

#include <vector>
#include <string>
#include "PacketQueue.h"
#include "StreamParser.h"

#include "moreuuids.h"

#include "LAVSplitter.h"

class CLAVOutputPin
  : public CBaseOutputPin
  , IMediaSeeking
  , protected CAMThread
{
public:
  CLAVOutputPin(std::vector<CMediaType>& mts, LPCWSTR pName, CBaseFilter *pFilter, CCritSec *pLock, HRESULT *phr, CBaseDemuxer::StreamType pinType = CBaseDemuxer::unknown,const char* container = "", int nBuffers = 0);
  CLAVOutputPin(LPCWSTR pName, CBaseFilter *pFilter, CCritSec *pLock, HRESULT *phr, CBaseDemuxer::StreamType pinType = CBaseDemuxer::unknown, const char* container = "", int nBuffers = 0);
  virtual ~CLAVOutputPin();

  DECLARE_IUNKNOWN;
  STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void** ppv);

  // IQualityControl
  STDMETHODIMP Notify(IBaseFilter* pSender, Quality q) { return E_NOTIMPL; }

  // CBaseOutputPin
  HRESULT CheckConnect(IPin* pPin);
  HRESULT DecideBufferSize(IMemAllocator* pAlloc, ALLOCATOR_PROPERTIES* pProperties);
  HRESULT CheckMediaType(const CMediaType* pmt);
  HRESULT GetMediaType(int iPosition, CMediaType* pmt);
  HRESULT Active();
  HRESULT Inactive();

  // IMediaSeeking
  STDMETHODIMP GetCapabilities(DWORD* pCapabilities);
  STDMETHODIMP CheckCapabilities(DWORD* pCapabilities);
  STDMETHODIMP IsFormatSupported(const GUID* pFormat);
  STDMETHODIMP QueryPreferredFormat(GUID* pFormat);
  STDMETHODIMP GetTimeFormat(GUID* pFormat);
  STDMETHODIMP IsUsingTimeFormat(const GUID* pFormat);
  STDMETHODIMP SetTimeFormat(const GUID* pFormat);
  STDMETHODIMP GetDuration(LONGLONG* pDuration);
  STDMETHODIMP GetStopPosition(LONGLONG* pStop);
  STDMETHODIMP GetCurrentPosition(LONGLONG* pCurrent);
  STDMETHODIMP ConvertTimeFormat(LONGLONG* pTarget, const GUID* pTargetFormat, LONGLONG Source, const GUID* pSourceFormat);
  STDMETHODIMP SetPositions(LONGLONG* pCurrent, DWORD dwCurrentFlags, LONGLONG* pStop, DWORD dwStopFlags);
  STDMETHODIMP GetPositions(LONGLONG* pCurrent, LONGLONG* pStop);
  STDMETHODIMP GetAvailable(LONGLONG* pEarliest, LONGLONG* pLatest);
  STDMETHODIMP SetRate(double dRate);
  STDMETHODIMP GetRate(double* pdRate);
  STDMETHODIMP GetPreroll(LONGLONG* pllPreroll);

  size_t QueueCount();
  HRESULT QueuePacket(Packet *pPacket);
  HRESULT QueueEndOfStream();
  bool IsDiscontinuous();

  DWORD GetStreamId() { return m_streamId; };
  void SetStreamId(DWORD newStreamId) { m_streamId = newStreamId; };

  void SetNewMediaTypes(std::vector<CMediaType> pmts) { CAutoLock lock(&m_csMT); m_mts = pmts; }
  void SendMediaType(CMediaType *mt) { CAutoLock lock(&m_csMT); m_newMT = mt;}

  BOOL IsVideoPin() { return m_pinType == CBaseDemuxer::video; }
  BOOL IsAudioPin() { return m_pinType == CBaseDemuxer::audio; }
  BOOL IsSubtitlePin(){ return m_pinType == CBaseDemuxer::subpic; }
  CBaseDemuxer::StreamType GetPinType() { return m_pinType; }

  HRESULT QueueFromParser(Packet *pPacket) { m_queue.Queue(pPacket); return S_OK; }

public:
  // Packet handling functions
  virtual HRESULT DeliverBeginFlush();
  virtual HRESULT DeliverEndFlush();

  virtual HRESULT DeliverNewSegment(REFERENCE_TIME tStart, REFERENCE_TIME tStop, double dRate);

protected:
  virtual HRESULT DeliverPacket(Packet *pPacket);

private:
  enum {CMD_EXIT};
  DWORD ThreadProc();

private:
  CCritSec m_csMT;
  std::vector<CMediaType> m_mts;
  CPacketQueue m_queue;

  std::string m_containerFormat;

  REFERENCE_TIME m_rtStart;

  // Flush control
  bool m_fFlushing, m_fFlushed;
  CAMEvent m_eEndFlush;

  HRESULT m_hrDeliver;

  int m_nBuffers;

  DWORD m_streamId;
  CMediaType *m_newMT;

  CBaseDemuxer::StreamType m_pinType;

  CStreamParser m_Parser;
};
