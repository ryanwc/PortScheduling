string: viz_port_file_path = file_path()++".html";

function string: portschedule_viz(array[int] of var int: westend,
  array[int] of var int:eastend,
  array[int] of var int:stack,
  array[int] of var int:endstack,
  array[int] of var int:reclaim,
  array[int] of var int:finish,
  array[int] of var int:which) =
  "%%%mzn-json:"++viz_port_file_path++"\n[\n"++
    concat(
    [join(", \n",["{\"westend\":\(westend[i]), "++
     "\"eastend\":\(eastend[i]), "++
     "\"stack\":\(stack[i]), "++
     "\"endstack\":\(endstack[i]), "++
     "\"reclaim\":\(reclaim[i]), "++
     "\"finish\":\(finish[i]), "++
     "\"which\":\(which[i])}" | i in index_set(westend)])])
    ++"]\n%%%mzn-json-end\n";

