//---------------------------------------------------------------------------
#ifndef DragDropH
#define DragDropH
//---------------------------------------------------------------------------
#include <windows.h>

void RegisterDropWindow(HWND hwnd, IDropTarget **ppDropTarget);

void UnregisterDropWindow(HWND hwnd, IDropTarget *pDropTarget);

HRESULT CreateDropSource(IDropSource **ppDropSource);

HRESULT CreateDataObject (FORMATETC *fmtetc, STGMEDIUM *stgmeds, UINT count, IDataObject **ppDataObject);

#endif
