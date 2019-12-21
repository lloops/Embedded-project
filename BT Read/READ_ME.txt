In the bt_master.ino, note that the four_hex[20 ? ][4] 2d array, the first parameter 20 is the maxm # of devices/responses you can get.

If anyone is setting inqm = 1, > or around 20, xx; you should change the 20 to a larger value, any parameter value that is well below 20, say 15, will work fine. If you set the 2nd parameter of inqm to 30, then do four_hex[a bit more than 30][4]. The 4 is never changed. 

Also, if you change the size of the 2d array, need to change the parameter value of the passed in 2d arr's size 
of: void convert_from_ascii(int arr[20][4], int num_response) as well.

Record/Screenshot any bugs you see.