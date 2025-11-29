open Lexer;;

type configuration = 
  Default of Commands.Default.config
  | Build of Commands.Build.config (* Possiblemnt : executable, test, examples*)
  | New of Commands.New.config
(* | Help of help_options
  | Clean of clean_options
  | Doc of doc_options
  | Watch of watch_options
*)

let configuration_of_args () =
  let argc = Array.length (Sys.argv) in
  if argc >= 2 then
    let tokens = List.rev (cli_tokens_of_args ()) in
    let options = List.rev (options_of_tokens tokens) in
    match options with
    | PosArg "new"::tl -> New(Commands.New.of_options tl)
    | PosArg "build"::tl -> Build(Commands.Build.of_options tl)
    | (PosArg _)::_ -> Default({version=false; help=true})
    | _ -> Default(Commands.Default.of_options options)
  else Default({version=false; help=true})
;;

let _ = 
  let config = configuration_of_args () in
  match config with
  | Default(default_opt) -> Commands.Default.execute default_opt
  | Build(build_opt) -> Commands.Build.execute build_opt
  | New(new_opt) -> Commands.New.execute new_opt
;;
