object QuickHelpForm: TQuickHelpForm
  Left = 611
  Top = 124
  HorzScrollBar.Visible = False
  VertScrollBar.Visible = False
  BorderIcons = [biSystemMenu]
  Caption = 'Quick Help'
  ClientHeight = 82
  ClientWidth = 464
  Color = clCream
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  FormStyle = fsStayOnTop
  KeyPreview = True
  OldCreateOrder = False
  OnKeyPress = FormKeyPress
  PixelsPerInch = 96
  TextHeight = 13
  object QuickHelpBox: TRichEdit
    Left = 0
    Top = 0
    Width = 464
    Height = 82
    Align = alClient
    BevelInner = bvNone
    BevelOuter = bvNone
    Font.Charset = ANSI_CHARSET
    Font.Color = clWindowText
    Font.Height = -12
    Font.Name = 'Courier New'
    Font.Style = []
    ParentFont = False
    PopupMenu = QuickHelpBoxMenu
    ReadOnly = True
    ScrollBars = ssBoth
    TabOrder = 0
    WordWrap = False
    Zoom = 100
  end
  object QuickHelpBoxMenu: TPopupMenu
    Left = 8
    Top = 8
    object QuickHelpBoxMenu_AutoPosition: TMenuItem
      AutoCheck = True
      Caption = 'Position Automatically'
    end
    object QuickHelpBoxMenu_N1: TMenuItem
      Caption = '-'
    end
    object QuickHelpBoxMenu_ChangeFont: TMenuItem
      Caption = 'Change Font...'
      OnClick = QuickHelpBoxMenu_ChangeFontClick
    end
  end
  object FontDlg: TFontDialog
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -11
    Font.Name = 'MS Sans Serif'
    Font.Style = []
    Left = 40
    Top = 8
  end
end
