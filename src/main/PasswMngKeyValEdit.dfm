object PasswMngKeyValDlg: TPasswMngKeyValDlg
  Left = 0
  Top = 0
  BorderIcons = [biSystemMenu, biMaximize]
  Caption = 'Key-value list editor'
  ClientHeight = 209
  ClientWidth = 473
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -14
  Font.Name = 'Tahoma'
  Font.Style = []
  OnShow = FormShow
  PixelsPerInch = 120
  DesignSize = (
    473
    209)
  TextHeight = 17
  object KeyValueGrid: TStringGrid
    Left = 0
    Top = 0
    Width = 473
    Height = 153
    Margins.Left = 4
    Margins.Top = 4
    Margins.Right = 4
    Margins.Bottom = 4
    Align = alTop
    Anchors = [akLeft, akTop, akRight, akBottom]
    ColCount = 2
    DefaultColWidth = 80
    DefaultRowHeight = 23
    FixedCols = 0
    Options = [goFixedVertLine, goFixedHorzLine, goVertLine, goHorzLine, goRangeSelect, goColSizing, goTabs]
    TabOrder = 0
    OnSelectCell = KeyValueGridSelectCell
    ExplicitWidth = 467
    ExplicitHeight = 152
    ColWidths = (
      189
      249)
  end
  object OKBtn: TButton
    Left = 262
    Top = 164
    Width = 94
    Height = 31
    Margins.Left = 4
    Margins.Top = 4
    Margins.Right = 4
    Margins.Bottom = 4
    Anchors = [akRight, akBottom]
    Caption = 'OK'
    Default = True
    ModalResult = 1
    TabOrder = 1
  end
  object CancelBtn: TButton
    Left = 364
    Top = 164
    Width = 94
    Height = 31
    Margins.Left = 4
    Margins.Top = 4
    Margins.Right = 4
    Margins.Bottom = 4
    Anchors = [akRight, akBottom]
    Caption = 'Cancel'
    ModalResult = 2
    TabOrder = 2
  end
end
