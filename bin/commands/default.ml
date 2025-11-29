open Exceptions;;
open Options;;

let version_string v =
    match v with
    | None -> "n/a"
    | Some v -> Build_info.V1.Version.to_string v

let help_file = Filename.concat File.help_dir "default.txt"

type config = {
  version:bool; (* default is [false] *)
  help:bool; (* default is [true]*)
}

let of_options = List.fold_left (fun r opt ->
  match opt with
  | Version -> {r with version=true}
  | Help -> {r with help=true}
  | _ -> raise Invalid_command_option
)
(* Default options when no commands : *)
{
  version=false;
  help=false
}
let execute opts =
  if opts.help 
  then File.print_text help_file
  else 
    if opts.version then begin
      let version = Build_info.V1.version () in
      Printf.printf "cpf (C Programmer's Friend) version %s\n" (version_string version)
    end else ()
