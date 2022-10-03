-- Example Lua script for Password Tech
-- Convert characters in a given password into leetspeak symbols
-- (with a probability of 50% for each character)

-- leetspeak alphabet
leetalpha = {
	["A"] = { "/-\\", "/\\", "4", "@" },
	["B"] = { "|3", "8", "|o" },
	["C"] = { "(", "<" },
	["D"] = { "|)", "o|", "|>", "<|" },
	["E"] = { "3" },
	["F"] = { "|=", "ph" },
	["G"] = { "9", "6", "&" },
	["H"] = { "|-|", "]-[", "}-{", "(-)", ")-(", "#" },
	["I"] = { "l", "1", "|", "!", "][" },
	["J"] = { "_|" },
	["K"] = { "|<", "/<", "\\<", "|{" },
	["L"] = { "|_", "|", "1" },
	["M"] = { "|\\/|", "/\\/\\", "|'|'|", "(\\/)", "/\\\\", "/|\\", "/v\\" },
	["N"] = { "|\\|", "/\\/", "|\\\\|", "/|/" },
	["O"] = { "0", "()", "[]", "{}" },
	["P"] = { "|°", "|>", "|*", "|D" },
	["Q"] = { "(,)", "kw" },
	["R"] = { "|2", "|Z", "|?" },
	["S"] = { "5", "$" },
	["T"] = { "+", "][", "7" },
	["U"] = { "|_|" },
	["V"] = { "|/", "\\|", "\\/", "\\'" },
	["W"] = { "\\/\\/", "\\|\\|", "|/|/", "\\|/", "\\^/" },
	["X"] = { "><", "}{" },
	["Y"] = { "'/", "°/" },
	["Z"] = { "2", "(\\)" }
}

log2=math.log(2)

function generate(passw_num, in_passw, in_entropy)
	local out_passw=""
	local out_entropy=in_entropy
	local i=1
	while i<=#in_passw do
		-- get next 8-bit character (octet)
		local c=string.byte(in_passw,i)
		
		-- convert letter into upper-case
		if c>=97 and c<=122 then
			c=65+c-97
		end
		
		-- get conversion alphabet from table
		local conv=leetalpha[string.char(c)]
		
		local r=0
		local j=i
		
		if conv then
			-- convert with 50% probability
			if pwtech_random(2)==1 then
				-- randomly select leetspeak symbol and append to password
				r=pwtech_random(#conv)
				out_passw=out_passw..conv[r]
				out_entropy=out_entropy+math.log(#conv)/log2
			end
			-- increment entropy by 1 bit due to 50% probability
			out_entropy=out_entropy+1
		end
		
		-- skip octets belonging to Unicode character units (up to 4 octets)
		if r==0 then
		    if c>=240 then j=i+3
			elseif c>=224 then j=i+2
			elseif c>=192 then j=i+1
			end
			out_passw=out_passw..string.sub(in_passw,i,j)
		end
		i=j+1
	end
	
	-- append converted password to original password and output this string
	return in_passw.." -> "..out_passw, out_entropy
end