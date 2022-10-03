-- Example Lua script for Password Tech
-- Generate permutation of numbers 1..N (N>1)
-- N is to be specified via the "Format password" string in the main window

script_flags=1
perm_num=0
log2=math.log(2)

function init(num_passw, dest, gen_flags, advanced_flags, num_chars, num_words, format_str)
	-- convert format string to number and store in global variable
	num=tonumber(format_str)
	-- avoid assignment of nil
	perm_num=num or 0
end

function generate(passw_num, in_passw, in_entropy)
	if perm_num>1 then
	    -- fill table with numbers in range 1..N
		local t={}
		for i=1,perm_num do
			t[i]=i
		end
		
		-- shuffle array elements (Fisher-Yates)
		-- and calculate entropy as log(N!)
		local out_entropy=0
		for i=perm_num,2,-1 do
			local r=pwtech_random(i)
			if r~=i then
				local temp=t[r]
				t[r]=t[i]
				t[i]=temp
			end
			out_entropy=out_entropy+math.log(i)
		end
		
		-- convert shuffled array into password string
		local out_passw=""
		for i=1,perm_num do
			out_passw=out_passw..tostring(t[i])
			if i<perm_num then out_passw=out_passw..", " end
		end
		
		return out_passw, out_entropy/log2
	end
	return "use format string to specify number >1", 0
end