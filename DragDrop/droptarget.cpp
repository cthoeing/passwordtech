//
//	DROPTARGET.CPP
//
//	By J Brown 2004
//
//	www.catch22.net
//
// modified by C.T. to provide Unicode support

//#define STRICT

#include "dragdrop.h"

void DropData(HWND hwnd, IDataObject *pDataObject, CLIPFORMAT clipFormat);

//
//	This is our definition of a class which implements
//  the IDropTarget interface
//
class CDropTarget : public IDropTarget
{
public:
	// IUnknown implementation
	HRESULT __stdcall QueryInterface (REFIID iid, void ** ppvObject);
	ULONG	__stdcall AddRef (void);
	ULONG	__stdcall Release (void);

	// IDropTarget implementation
	HRESULT __stdcall DragEnter (IDataObject * pDataObject, DWORD grfKeyState, POINTL pt, DWORD * pdwEffect);
	HRESULT __stdcall DragOver (DWORD grfKeyState, POINTL pt, DWORD * pdwEffect);
	HRESULT __stdcall DragLeave (void);
	HRESULT __stdcall Drop (IDataObject * pDataObject, DWORD grfKeyState, POINTL pt, DWORD * pdwEffect);

	// Constructor
	CDropTarget(HWND hwnd);
	~CDropTarget();

private:

	// internal helper function
	DWORD DropEffect(DWORD grfKeyState, POINTL pt, DWORD dwAllowed);
	bool  QueryDataObject(IDataObject *pDataObject);


	// Private member variables
	LONG	m_lRefCount;
	HWND	m_hWnd;
	bool    m_fAllowDrop;
        CLIPFORMAT m_clipFormat;

	IDataObject *m_pDataObject;

};

//
//	Constructor for the CDropTarget class
//
CDropTarget::CDropTarget(HWND hwnd)
{
	m_lRefCount  = 1;
	m_hWnd       = hwnd;
	m_fAllowDrop = false;
	m_clipFormat = CF_UNICODETEXT;
}

//
//	Destructor for the CDropTarget class
//
CDropTarget::~CDropTarget()
{

}

//
//	Position the edit control's caret under the mouse
//
void PositionCursor(HWND hwndEdit, POINTL pt)
{
	DWORD curpos;

	// get the character position of mouse
	ScreenToClient(hwndEdit, (POINT *)&pt);
	curpos = SendMessage(hwndEdit, EM_CHARFROMPOS, 0, MAKELPARAM(pt.x, pt.y));

	// set cursor position
	SendMessage(hwndEdit, EM_SETSEL, LOWORD(curpos), LOWORD(curpos));
}

//
//	IUnknown::QueryInterface
//
HRESULT __stdcall CDropTarget::QueryInterface (REFIID iid, void ** ppvObject)
{
	if(iid == IID_IDropTarget || iid == IID_IUnknown)
	{
		AddRef();
		*ppvObject = this;
		return S_OK;
	}
	else
	{
		*ppvObject = 0;
		return E_NOINTERFACE;
	}
}

//
//	IUnknown::AddRef
//
ULONG __stdcall CDropTarget::AddRef(void)
{
	return InterlockedIncrement(&m_lRefCount);
}

//
//	IUnknown::Release
//
ULONG __stdcall CDropTarget::Release(void)
{
	LONG count = InterlockedDecrement(&m_lRefCount);

	if(count == 0)
	{
		delete this;
		return 0;
	}
	else
	{
		return count;
	}
}

//
//	QueryDataObject private helper routine
//
bool CDropTarget::QueryDataObject(IDataObject *pDataObject)
{
	FORMATETC fmtetc = { m_clipFormat, 0, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };

	// does the data object support CF_TEXT using a HGLOBAL?
	return pDataObject->QueryGetData(&fmtetc) == S_OK ? true : false;
}

//
//	DropEffect private helper routine
//
DWORD CDropTarget::DropEffect(DWORD grfKeyState, POINTL pt, DWORD dwAllowed)
{
	DWORD dwEffect = 0;

	// 1. check "pt" -> do we allow a drop at the specified coordinates?

	// 2. work out that the drop-effect should be based on grfKeyState
	if(grfKeyState & MK_CONTROL)
	{
		dwEffect = dwAllowed & DROPEFFECT_COPY;
	}
	else if(grfKeyState & MK_SHIFT)
	{
		dwEffect = dwAllowed & DROPEFFECT_MOVE;
	}

	// 3. no key-modifiers were specified (or drop effect not allowed), so
	//    base the effect on those allowed by the dropsource
	if(dwEffect == 0)
	{
		if(dwAllowed & DROPEFFECT_COPY) dwEffect = DROPEFFECT_COPY;
		if(dwAllowed & DROPEFFECT_MOVE) dwEffect = DROPEFFECT_MOVE;
	}

	return dwEffect;
}


//
//	IDropTarget::DragEnter
//
//
//
HRESULT __stdcall CDropTarget::DragEnter(IDataObject * pDataObject, DWORD grfKeyState, POINTL pt, DWORD * pdwEffect)
{
	// does the dataobject contain data we want?
        m_fAllowDrop = !(GetWindowLong(m_hWnd, GWL_STYLE) & ES_READONLY) &&
                QueryDataObject(pDataObject);

	if(m_fAllowDrop)
	{
		// get the dropeffect based on keyboard state
		*pdwEffect = DropEffect(grfKeyState, pt, *pdwEffect);

		SetFocus(m_hWnd);

		PositionCursor(m_hWnd, pt);
	}
	else
	{
		*pdwEffect = DROPEFFECT_NONE;
	}

	return S_OK;
}

//
//	IDropTarget::DragOver
//
//
//
HRESULT __stdcall CDropTarget::DragOver(DWORD grfKeyState, POINTL pt, DWORD * pdwEffect)
{
	if(m_fAllowDrop)
	{
		*pdwEffect = DropEffect(grfKeyState, pt, *pdwEffect);
		PositionCursor(m_hWnd, pt);
	}
	else
	{
		*pdwEffect = DROPEFFECT_NONE;
	}

	return S_OK;
}

//
//	IDropTarget::DragLeave
//
HRESULT __stdcall CDropTarget::DragLeave(void)
{
	return S_OK;
}

//
//	IDropTarget::Drop
//
//
HRESULT __stdcall CDropTarget::Drop(IDataObject * pDataObject, DWORD grfKeyState, POINTL pt, DWORD * pdwEffect)
{
	PositionCursor(m_hWnd, pt);

	if(m_fAllowDrop)
	{
		DropData(m_hWnd, pDataObject, m_clipFormat);

		*pdwEffect = DropEffect(grfKeyState, pt, *pdwEffect);
	}
	else
	{
		*pdwEffect = DROPEFFECT_NONE;
	}

	return S_OK;
}

void DropData(HWND hwnd, IDataObject *pDataObject, CLIPFORMAT clipFormat)
{
	// construct a FORMATETC object
	FORMATETC fmtetc = { clipFormat, 0, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
	STGMEDIUM stgmed;

	// See if the dataobject contains any TEXT stored as a HGLOBAL
	if(pDataObject->QueryGetData(&fmtetc) == S_OK)
	{
		// Yippie! the data is there, so go get it!
		if(pDataObject->GetData(&fmtetc, &stgmed) == S_OK)
		{
			// we asked for the data as a HGLOBAL, so access it appropriately
			PVOID data = GlobalLock(stgmed.hGlobal);

						if (clipFormat == CF_UNICODETEXT)
								SetWindowTextW(hwnd, (wchar_t*) data);
						else
								SetWindowTextA(hwnd, (char*) data);

			GlobalUnlock(stgmed.hGlobal);

			// release the data using the COM API
			ReleaseStgMedium(&stgmed);
		}
	}
}

void RegisterDropWindow(HWND hwnd, IDropTarget **ppDropTarget)
{
	CDropTarget *pDropTarget = new CDropTarget(hwnd);

	// acquire a strong lock
	CoLockObjectExternal(pDropTarget, TRUE, FALSE);

	// tell OLE that the window is a drop target
	RegisterDragDrop(hwnd, pDropTarget);

	*ppDropTarget = pDropTarget;
}

void UnregisterDropWindow(HWND hwnd, IDropTarget *pDropTarget)
{
	// remove drag+drop
	RevokeDragDrop(hwnd);

	// remove the strong lock
	CoLockObjectExternal(pDropTarget, FALSE, TRUE);

	// release our own reference
	pDropTarget->Release();
}
