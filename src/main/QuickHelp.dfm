object QuickHelpForm: TQuickHelpForm
  Left = 611
  Top = 124
  HorzScrollBar.Visible = False
  VertScrollBar.Visible = False
  BorderIcons = [biSystemMenu]
  Caption = 'Quick Help'
  ClientHeight = 100
  ClientWidth = 570
  Color = clCream
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -14
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  FormStyle = fsStayOnTop
  KeyPreview = True
  OnKeyPress = FormKeyPress
  PixelsPerInch = 120
  TextHeight = 16
  object QuickHelpBox: TRichEdit
    Left = 0
    Top = 0
    Width = 570
    Height = 100
    Margins.Left = 4
    Margins.Top = 4
    Margins.Right = 4
    Margins.Bottom = 4
    Align = alClient
    BevelInner = bvNone
    BevelOuter = bvNone
    Font.Charset = ANSI_CHARSET
    Font.Color = clWindowText
    Font.Height = -13
    Font.Name = 'Consolas'
    Font.Style = []
    ParentFont = False
    PopupMenu = QuickHelpBoxMenu
    ReadOnly = True
    ScrollBars = ssBoth
    TabOrder = 0
    WordWrap = False
    ExplicitWidth = 564
    ExplicitHeight = 99
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
