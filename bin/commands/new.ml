open Exceptions;;
open Options;;

let help_file = Filename.concat File.help_dir "new.txt"

type config = {
  help:bool;
  name:string;
  lang:lang;
}

let of_options = List.fold_left (fun r opt ->
  match opt with
  | Help -> {r with help=true}
  | Name s -> {r with name=s}
  | Lang l -> {r with lang=l}
  | _ -> raise Invalid_command_option
)
(* Default options when new command : *)
{
  help=false;
  name="";
  lang=C
}
let execute opts =
  if opts.help then File.print_text help_file
  else (
    let project_dir = Filename.concat (Sys.getcwd ()) opts.name in
    let include_dir = Filename.concat project_dir "include" in
    let src_dir = Filename.concat project_dir "src" in
    let tests_dir = Filename.concat project_dir "tests" in
    let ldscripts_dir = Filename.concat project_dir "ldscripts" in
    let lib_dir = Filename.concat project_dir "lib" in
    begin
      try (
        Sys.mkdir project_dir 0o777;
        Sys.mkdir include_dir 0o777;
        Sys.mkdir src_dir 0o777;
        Sys.mkdir tests_dir 0o777;
        Sys.mkdir ldscripts_dir 0o777;
        Sys.mkdir lib_dir 0o777
      ) with
      | Sys_error _ -> 
          if Sys.file_exists project_dir 
          then ()
          else (Printf.printf "Erreur inconnue."; failwith "")
    end;
    File.copy_text 
      (Filename.concat project_dir "Makefile")
      (Filename.concat File.data_dir "Makefile");
    (match opts.lang with
    | C -> File.copy_text (Filename.concat src_dir "main.c") (Filename.concat File.data_dir "main.c")
    | CPP -> File.copy_text (Filename.concat src_dir "main.cpp") (Filename.concat File.data_dir "main.cpp"));
    File.copy_text (Filename.concat include_dir "unittest.h") (Filename.concat File.data_dir "unittest.h");
    File.copy_text (Filename.concat tests_dir "some_name.test.c") (Filename.concat File.data_dir "some_name.test.c");
    File.copy_text (Filename.concat ldscripts_dir "link.template.ld") (Filename.concat File.data_dir "link.template.ld")
  )
