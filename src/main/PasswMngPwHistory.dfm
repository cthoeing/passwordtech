object PasswHistoryDlg: TPasswHistoryDlg
  Left = 0
  Top = 0
  BorderIcons = [biSystemMenu, biMaximize]
  Caption = 'Password History'
  ClientHeight = 239
  ClientWidth = 386
  Color = clBtnFace
  Constraints.MinHeight = 229
  Constraints.MinWidth = 316
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -14
  Font.Name = 'Tahoma'
  Font.Style = []
  OnShow = FormShow
  PixelsPerInch = 120
  DesignSize = (
    386
    239)
  TextHeight = 17
  object EnableHistoryCheck: TCheckBox
    Left = 13
    Top = 11
    Width = 220
    Height = 21
    Margins.Left = 4
    Margins.Top = 4
    Margins.Right = 4
    Margins.Bottom = 4
    Anchors = [akLeft, akTop, akRight]
    Caption = 'Keep last passwords - up to:'
    TabOrder = 0
    OnClick = EnableHistoryCheckClick
    ExplicitWidth = 214
  end
  object HistorySizeBox: TEdit
    Tag = 6
    Left = 298
    Top = 9
    Width = 51
    Height = 25
    Margins.Left = 4
    Margins.Top = 4
    Margins.Right = 4
    Margins.Bottom = 4
    TabOrder = 1
    Text = '1'
  end
  object HistorySizeSpinBtn: TUpDown
    Tag = 6
    Left = 349
    Top = 9
    Width = 20
    Height = 25
    Margins.Left = 4
    Margins.Top = 4
    Margins.Right = 4
    Margins.Bottom = 4
    Associate = HistorySizeBox
    Min = 1
    Position = 1
    TabOrder = 2
  end
  object HistoryView: TListView
    Tag = 15
    Left = 13
    Top = 44
    Width = 359
    Height = 139
    Margins.Left = 4
    Margins.Top = 4
    Margins.Right = 4
    Margins.Bottom = 4
    Columns = <
      item
        Caption = 'Set Date/Time'
        Width = 63
      end
      item
        Caption = 'Password'
        Width = 63
      end>
    MultiSelect = True
    ReadOnly = True
    RowSelect = True
    TabOrder = 3
    ViewStyle = vsReport
    OnKeyDown = HistoryViewKeyDown
  end
  object ClearBtn: TButton
    Tag = 9
    Left = 13
    Top = 195
    Width = 71
    Height = 31
    Margins.Left = 4
    Margins.Top = 4
    Margins.Right = 4
    Margins.Bottom = 4
    Caption = 'Clear'
    TabOrder = 4
    OnClick = ClearBtnClick
  end
  object CopyBtn: TButton
    Tag = 9
    Left = 88
    Top = 195
    Width = 71
    Height = 31
    Margins.Left = 4
    Margins.Top = 4
    Margins.Right = 4
    Margins.Bottom = 4
    Caption = 'Copy'
    TabOrder = 5
    OnClick = CopyBtnClick
  end
  object OKBtn: TButton
    Tag = 12
    Left = 180
    Top = 195
    Width = 92
    Height = 31
    Margins.Left = 4
    Margins.Top = 4
    Margins.Right = 4
    Margins.Bottom = 4
    Caption = 'OK'
    Default = True
    ModalResult = 1
    TabOrder = 6
  end
  object CancelBtn: TButton
    Tag = 12
    Left = 280
    Top = 195
    Width = 92
    Height = 31
    Margins.Left = 4
    Margins.Top = 4
    Margins.Right = 4
    Margins.Bottom = 4
    Caption = 'Cancel'
    ModalResult = 2
    TabOrder = 7
  end
end
