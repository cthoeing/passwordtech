object CharSetBuilderForm: TCharSetBuilderForm
  Left = 0
  Top = 0
  BorderIcons = [biSystemMenu]
  Caption = 'Character Set Builder'
  ClientHeight = 570
  ClientWidth = 503
  Color = clBtnFace
  Font.Charset = ANSI_CHARSET
  Font.Color = clWindowText
  Font.Height = -15
  Font.Name = 'Tahoma'
  Font.Style = []
  OnShow = FormShow
  PixelsPerInch = 120
  TextHeight = 18
  object CharSetSelGroup: TGroupBox
    Tag = 7
    Left = 10
    Top = 15
    Width = 481
    Height = 306
    Margins.Left = 4
    Margins.Top = 4
    Margins.Right = 4
    Margins.Bottom = 4
    Caption = 'Character set selection'
    TabOrder = 0
    DesignSize = (
      481
      306)
    object FromLbl: TLabel
      Left = 38
      Top = 187
      Width = 39
      Height = 18
      Margins.Left = 4
      Margins.Top = 4
      Margins.Right = 4
      Margins.Bottom = 4
      Caption = 'From:'
      Enabled = False
    end
    object ToLbl: TLabel
      Left = 154
      Top = 187
      Width = 18
      Height = 18
      Margins.Left = 4
      Margins.Top = 4
      Margins.Right = 4
      Margins.Bottom = 4
      Caption = 'to:'
      Enabled = False
    end
    object LowerCaseCheck: TCheckBox
      Left = 10
      Top = 26
      Width = 234
      Height = 27
      Margins.Left = 5
      Margins.Top = 5
      Margins.Right = 5
      Margins.Bottom = 5
      Anchors = [akLeft, akTop, akRight]
      Caption = 'Lower-case letters (a-z)'
      TabOrder = 0
      OnClick = LowerCaseCheckClick
    end
    object UpperCaseCheck: TCheckBox
      Left = 10
      Top = 58
      Width = 234
      Height = 26
      Margins.Left = 5
      Margins.Top = 5
      Margins.Right = 5
      Margins.Bottom = 5
      Anchors = [akLeft, akTop, akRight]
      Caption = 'Upper-case letters (A-Z)'
      TabOrder = 4
      OnClick = UpperCaseCheckClick
    end
    object DigitsCheck: TCheckBox
      Left = 10
      Top = 89
      Width = 234
      Height = 26
      Margins.Left = 5
      Margins.Top = 5
      Margins.Right = 5
      Margins.Bottom = 5
      Anchors = [akLeft, akTop, akRight]
      Caption = 'Digits (0-9)'
      TabOrder = 8
      OnClick = DigitsCheckClick
    end
    object SpecialSymbolsCheck: TCheckBox
      Left = 10
      Top = 120
      Width = 234
      Height = 26
      Margins.Left = 5
      Margins.Top = 5
      Margins.Right = 5
      Margins.Bottom = 5
      Anchors = [akLeft, akTop, akRight]
      Caption = 'Special symbols'
      TabOrder = 12
      OnClick = SpecialSymbolsCheckClick
    end
    object CharactersRangeCheck: TCheckBox
      Left = 10
      Top = 151
      Width = 234
      Height = 27
      Margins.Left = 5
      Margins.Top = 5
      Margins.Right = 5
      Margins.Bottom = 5
      Anchors = [akLeft, akTop, akRight]
      Caption = 'Characters from range:'
      TabOrder = 16
      OnClick = CharactersRangeCheckClick
    end
    object AdditionalCharsCheck: TCheckBox
      Left = 10
      Top = 222
      Width = 250
      Height = 27
      Margins.Left = 5
      Margins.Top = 5
      Margins.Right = 5
      Margins.Bottom = 5
      Caption = 'Additional characters:'
      TabOrder = 22
      OnClick = AdditionalCharsCheckClick
    end
    object FromBox: TEdit
      Left = 95
      Top = 184
      Width = 51
      Height = 26
      Margins.Left = 4
      Margins.Top = 4
      Margins.Right = 4
      Margins.Bottom = 4
      Enabled = False
      MaxLength = 1
      TabOrder = 20
      OnChange = CharSetParamChange
    end
    object ToBox: TEdit
      Left = 190
      Top = 184
      Width = 51
      Height = 26
      Margins.Left = 4
      Margins.Top = 4
      Margins.Right = 4
      Margins.Bottom = 4
      Enabled = False
      MaxLength = 1
      TabOrder = 21
      OnChange = CharSetParamChange
    end
    object AdditionalCharsBox: TEdit
      Left = 32
      Top = 253
      Width = 221
      Height = 26
      Margins.Left = 4
      Margins.Top = 4
      Margins.Right = 4
      Margins.Bottom = 4
      Anchors = [akLeft, akTop, akRight]
      Enabled = False
      TabOrder = 26
      OnChange = CharSetParamChange
    end
    object NumBox1: TEdit
      Left = 269
      Top = 26
      Width = 61
      Height = 26
      Margins.Left = 4
      Margins.Top = 4
      Margins.Right = 4
      Margins.Bottom = 4
      Enabled = False
      TabOrder = 1
      Text = '0'
      OnChange = CharSetParamChange
    end
    object NumSpinBtn1: TUpDown
      Left = 330
      Top = 26
      Width = 20
      Height = 26
      Margins.Left = 4
      Margins.Top = 4
      Margins.Right = 4
      Margins.Bottom = 4
      Associate = NumBox1
      Enabled = False
      TabOrder = 2
    end
    object AtLeastCheck1: TCheckBox
      Left = 358
      Top = 29
      Width = 111
      Height = 21
      Margins.Left = 4
      Margins.Top = 4
      Margins.Right = 4
      Margins.Bottom = 4
      Caption = 'At least'
      Enabled = False
      TabOrder = 3
      OnClick = CharSetParamChange
    end
    object NumBox2: TEdit
      Left = 269
      Top = 58
      Width = 61
      Height = 26
      Margins.Left = 4
      Margins.Top = 4
      Margins.Right = 4
      Margins.Bottom = 4
      Enabled = False
      TabOrder = 5
      Text = '0'
      OnChange = CharSetParamChange
    end
    object NumSpinBtn2: TUpDown
      Left = 330
      Top = 58
      Width = 20
      Height = 26
      Margins.Left = 4
      Margins.Top = 4
      Margins.Right = 4
      Margins.Bottom = 4
      Associate = NumBox2
      Enabled = False
      TabOrder = 6
    end
    object AtLeastCheck2: TCheckBox
      Left = 358
      Top = 61
      Width = 111
      Height = 21
      Margins.Left = 4
      Margins.Top = 4
      Margins.Right = 4
      Margins.Bottom = 4
      Caption = 'At least'
      Enabled = False
      TabOrder = 7
      OnClick = CharSetParamChange
    end
    object NumBox3: TEdit
      Left = 269
      Top = 89
      Width = 61
      Height = 26
      Margins.Left = 4
      Margins.Top = 4
      Margins.Right = 4
      Margins.Bottom = 4
      Enabled = False
      TabOrder = 9
      Text = '0'
      OnChange = CharSetParamChange
    end
    object NumSpinBtn3: TUpDown
      Left = 330
      Top = 89
      Width = 20
      Height = 26
      Margins.Left = 4
      Margins.Top = 4
      Margins.Right = 4
      Margins.Bottom = 4
      Associate = NumBox3
      Enabled = False
      TabOrder = 10
    end
    object AtLeastCheck3: TCheckBox
      Left = 358
      Top = 92
      Width = 111
      Height = 21
      Margins.Left = 4
      Margins.Top = 4
      Margins.Right = 4
      Margins.Bottom = 4
      Caption = 'At least'
      Enabled = False
      TabOrder = 11
      OnClick = CharSetParamChange
    end
    object NumBox4: TEdit
      Left = 269
      Top = 120
      Width = 61
      Height = 26
      Margins.Left = 4
      Margins.Top = 4
      Margins.Right = 4
      Margins.Bottom = 4
      Enabled = False
      TabOrder = 13
      Text = '0'
      OnChange = CharSetParamChange
    end
    object NumSpinBtn4: TUpDown
      Left = 330
      Top = 120
      Width = 20
      Height = 26
      Margins.Left = 4
      Margins.Top = 4
      Margins.Right = 4
      Margins.Bottom = 4
      Associate = NumBox4
      Enabled = False
      TabOrder = 14
    end
    object AtLeastCheck4: TCheckBox
      Left = 358
      Top = 123
      Width = 111
      Height = 21
      Margins.Left = 4
      Margins.Top = 4
      Margins.Right = 4
      Margins.Bottom = 4
      Caption = 'At least'
      Enabled = False
      TabOrder = 15
      OnClick = CharSetParamChange
    end
    object NumBox5: TEdit
      Left = 269
      Top = 151
      Width = 61
      Height = 26
      Margins.Left = 4
      Margins.Top = 4
      Margins.Right = 4
      Margins.Bottom = 4
      Enabled = False
      TabOrder = 17
      Text = '0'
      OnChange = CharSetParamChange
    end
    object NumSpinBtn5: TUpDown
      Left = 330
      Top = 151
      Width = 20
      Height = 26
      Margins.Left = 4
      Margins.Top = 4
      Margins.Right = 4
      Margins.Bottom = 4
      Associate = NumBox5
      Enabled = False
      TabOrder = 18
    end
    object AtLeastCheck5: TCheckBox
      Left = 358
      Top = 154
      Width = 111
      Height = 21
      Margins.Left = 4
      Margins.Top = 4
      Margins.Right = 4
      Margins.Bottom = 4
      Caption = 'At least'
      Enabled = False
      TabOrder = 19
      OnClick = CharSetParamChange
    end
    object NumBox6: TEdit
      Left = 269
      Top = 219
      Width = 61
      Height = 26
      Margins.Left = 4
      Margins.Top = 4
      Margins.Right = 4
      Margins.Bottom = 4
      Enabled = False
      TabOrder = 23
      Text = '0'
      OnChange = CharSetParamChange
    end
    object NumSpinBtn6: TUpDown
      Left = 330
      Top = 219
      Width = 20
      Height = 26
      Margins.Left = 4
      Margins.Top = 4
      Margins.Right = 4
      Margins.Bottom = 4
      Associate = NumBox6
      Enabled = False
      TabOrder = 24
    end
    object AtLeastCheck6: TCheckBox
      Left = 358
      Top = 222
      Width = 111
      Height = 21
      Margins.Left = 4
      Margins.Top = 4
      Margins.Right = 4
      Margins.Bottom = 4
      Caption = 'At least'
      Enabled = False
      TabOrder = 25
      OnClick = CharSetParamChange
    end
    object ResetBtn: TButton
      Left = 368
      Top = 266
      Width = 101
      Height = 31
      Margins.Left = 4
      Margins.Top = 4
      Margins.Right = 4
      Margins.Bottom = 4
      Anchors = [akTop, akRight]
      Caption = 'Reset'
      TabOrder = 27
      OnClick = ResetBtnClick
    end
  end
  object ResultGroup: TGroupBox
    Tag = 7
    Left = 10
    Top = 488
    Width = 481
    Height = 72
    Margins.Left = 4
    Margins.Top = 4
    Margins.Right = 4
    Margins.Bottom = 4
    Caption = 'Resulting character set definition'
    TabOrder = 1
    DesignSize = (
      481
      72)
    object ResultBox: TEdit
      Left = 10
      Top = 30
      Width = 350
      Height = 26
      Margins.Left = 4
      Margins.Top = 4
      Margins.Right = 4
      Margins.Bottom = 4
      Anchors = [akLeft, akTop, akRight]
      Color = clBtnFace
      ReadOnly = True
      TabOrder = 0
    end
    object ApplyBtn: TButton
      Left = 368
      Top = 27
      Width = 101
      Height = 31
      Margins.Left = 4
      Margins.Top = 4
      Margins.Right = 4
      Margins.Bottom = 4
      Anchors = [akTop, akRight]
      Caption = 'Apply'
      TabOrder = 1
      OnClick = ApplyBtnClick
    end
  end
  object TagGroup: TGroupBox
    Tag = 7
    Left = 10
    Top = 409
    Width = 481
    Height = 71
    Margins.Left = 4
    Margins.Top = 4
    Margins.Right = 4
    Margins.Bottom = 4
    Caption = 'Tag/comment'
    TabOrder = 3
    object TagBox: TEdit
      Tag = 7
      Left = 10
      Top = 30
      Width = 458
      Height = 26
      Margins.Left = 4
      Margins.Top = 4
      Margins.Right = 4
      Margins.Bottom = 4
      TabOrder = 0
      OnChange = CharSetParamChange
    end
  end
  object ExcludeCharsGroup: TGroupBox
    Tag = 7
    Left = 10
    Top = 330
    Width = 481
    Height = 71
    Margins.Left = 4
    Margins.Top = 4
    Margins.Right = 4
    Margins.Bottom = 4
    Caption = 'Exclude characters'
    TabOrder = 2
    object ExcludeCharsBox: TEdit
      Tag = 7
      Left = 10
      Top = 30
      Width = 457
      Height = 26
      Margins.Left = 4
      Margins.Top = 4
      Margins.Right = 4
      Margins.Bottom = 4
      TabOrder = 0
      OnChange = CharSetParamChange
    end
  end
end
