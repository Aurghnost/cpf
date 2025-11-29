type lang = C
  | CPP

type cli_option =
  | Quiet
  | Version
  | Help
  | Name of string
  | Release
  | Lang of lang
  | PosArg of string
  | Binary
  | Library
  | Tests
  | Examples
  | Config of string
