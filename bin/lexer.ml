open Exceptions;;
open Options;;
type cli_token =
  | ShortOpt of string
  | LongOpt of string
  | Arg of string

let cli_tokens_of_args () : cli_token list =
  let argc = Array.length Sys.argv in 
  let rec aux i acc =
    if i >= argc then acc
    else 
      let s = Sys.argv.(i) in
      let n = String.length s in
      if n < 2 || s.[0] <> '-' then
        match s.[0] with
        | '-' -> raise (Invalid_token s)
        | _ -> aux (i+1) ((Arg s)::acc)
      else
        if s.[1] <> '-' then aux (i+1) (ShortOpt (String.sub s 1 (n-1))::acc)
        else 
          match String.index_opt s '=' with
          | None -> aux (i+1) (LongOpt (String.sub s 2 (n- 2))::acc)
          | Some j ->
              let a = String.sub s (j+1) (n-(j+1)) in
              aux (i+1) (Arg a::(LongOpt (String.sub s 2 (j-2)))::acc)
  in
  aux 1 []

let options_of_tokens (tokens:cli_token list) =
  let get_opt_arg lst =
    match lst with
      | Arg s::ttl -> s, ttl
      | _ -> raise Argument_required
  in
  let rec aux lst acc = match lst with
  | [] -> acc
  | t::tl ->begin
      match t with
      | Arg s -> aux tl (PosArg s::acc)
      | LongOpt "quiet" -> aux tl (Quiet::acc)
      | LongOpt "version" -> aux tl (Version::acc)
      | LongOpt "release" -> aux tl (Release::acc)
      | LongOpt "name" -> 
          let arg, ttl = get_opt_arg tl in 
          aux ttl (Name arg::acc)
      | LongOpt "config" ->
          let arg, ttl = get_opt_arg tl in
          aux ttl (Config arg::acc)
      | LongOpt "lib" -> aux tl (Library::acc)
      | LongOpt "tests" -> aux tl (Tests::acc)
      | LongOpt "examples" -> aux tl (Examples::acc)
      | LongOpt "bin" -> aux tl (Binary::acc)
      | LongOpt "help" -> aux tl (Help::acc)
      | LongOpt "lang" ->
          let arg, ttl = get_opt_arg tl in
          (match String.lowercase_ascii arg with
          | "c" -> aux ttl (Lang C::acc)
          | "c++" | "cpp" -> aux ttl (Lang CPP::acc)
          | _ -> raise (Unknown_arg_value arg))
      | LongOpt s | ShortOpt s -> raise (Unknown_option s)
    end
  in
  aux tokens []

let rec display_tokens = function
  | [] -> ()
  | Arg s::tl -> Printf.printf "A(%s)\n" s; display_tokens tl
  | ShortOpt s::tl -> Printf.printf "SO(%s)\n" s; display_tokens tl
  | LongOpt s::tl -> Printf.printf "LO(%s)\n" s; display_tokens tl


