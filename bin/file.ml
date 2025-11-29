
(* let executable_dir = Filename.dirname Sys.executable_name *)
let executable_dir = Filename.current_dir_name
let data_dir = Filename.concat executable_dir "data"
let help_dir = Filename.concat executable_dir "help"

let print_text filename =
  let ic = open_in filename in
  try (
    while true do
      let line = input_line ic in
      print_endline line;
    done;
    ()
  ) with
  | End_of_file -> close_in ic
  | e -> raise e

let copy_text dest src =
  let ic = open_in src in
  let oc = open_out dest in
  try (
    while true do
      let line = input_line ic in
      Printf.fprintf oc "%s\n" line
    done;
  ) with
  | End_of_file -> close_in ic; close_out oc
  | e -> raise e
