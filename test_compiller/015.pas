program sort;
const
    m = 10;

var
    arr: array[0..m] of integer;
    i, j, k: integer;

begin

    write('Array: ');
    for i := 0 to m do begin
        arr[i] := m-i;
        write(arr[i], #10);
    end;


    for i := 0 to m-1 do
        for j := 0 to m-1-i do
            if arr[j] > arr[j+1] then begin
                k := arr[j];
                arr[j] := arr[j+1];
                arr[j+1] := k
            end;

    write ('Sort array: ');
    for i := 0 to m do
        write (arr[i], #10);

end.
