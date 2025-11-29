open Exceptions;;
open Options;;

let help_file = Filename.concat File.help_dir "build.txt"

type config = {
  quiet:bool;
  help:bool;
  release:bool;
  library:bool;
  binary:bool;
  tests:bool;
  examples:bool;
  config_file:string;
}

let of_options = List.fold_left (fun r opt ->
  match opt with
  | Quiet -> {r with quiet=true}
  | Help -> {r with help=true}
  | Release -> {r with release=true}
  | Library -> {r with library=true}
  | Binary -> {r with binary=true}
  | Tests -> {r with tests=true}
  | Examples -> {r with examples=true}
  | Config s -> {r with config_file=s}
  | _ -> raise Invalid_command_option
)
(* Default options when build command : *)
{
  quiet=false;
  help=false;
  release=false;
  library=false;
  binary=false;
  tests=false;
  examples=false;
  config_file="";
}
let execute opts =
  if opts.help then File.print_text help_file
  else (
  )
