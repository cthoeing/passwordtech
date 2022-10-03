-- Example Lua script for Password Tech
-- Convert characters of generated password into words from spelling alphabet
-- (alfa, bravo, charlie, etc.)
-- Upper-case letters are preceded by the word "upper"

spelling_alpha = {
	["A"] = "alfa",
	["B"] = "bravo",
	["C"] = "charlie",
	["D"] = "delta",
	["E"] = "echo",
	["F"] = "foxtrot",
	["G"] = "golf",
	["H"] = "hotel",
	["I"] = "india",
	["J"] = "juliett",
	["K"] = "kilo",
	["L"] = "lima",
	["M"] = "mike",
	["N"] = "november",
	["O"] = "oscar",
	["P"] = "papa",
	["Q"] = "quebec",
	["R"] = "romeo",
	["S"] = "sierra",
	["T"] = "tango",
	["U"] = "uniform",
	["V"] = "victor",
	["W"] = "whiskey",
	["X"] = "xray",
	["Y"] = "yankee",
	["Z"] = "zulu",
	["1"] = "one",
	["2"] = "two",
	["3"] = "three",
	["4"] = "four",
	["5"] = "five",
	["6"] = "six",
	["7"] = "seven",
	["8"] = "eight",
	["9"] = "nine",
	["0"] = "zero"
}

function generate(passw_num, in_passw, in_entropy)
	if not in_passw then
		return "empty password", 0
	end
	local out_passw=""
	local plen=#in_passw
	local i=1
	while i<=plen do
		-- get next octet
		local c=string.byte(in_passw,i)
		-- upper-case letter?
		local uc=c>=65 and c<=90
		-- convert lower-case letter to upper-case
		if c>=97 and c<=122 then
			c=c-97+65
		end
		-- get word from spelling alphabet
		-- (nil if octet not contained in table)
		local conv=spelling_alpha[string.char(c)]
		local j=i
		if conv then
			-- append word to output string,
			-- include string "upper" if letter is upper-case
			if uc then
				out_passw=out_passw.."upper "
			end
			out_passw=out_passw..conv
		else
			-- skip non-ASCII octets
			if c>=240 then j=i+3
			elseif c>=224 then j=i+2
			elseif c>=192 then j=i+1
			end
			out_passw=out_passw.."\""..string.sub(in_passw,i,j).."\""
		end
		if j<plen then
			out_passw=out_passw.." "
		end
		i=j+1
	end
	-- return both original and converted password
	return in_passw.." -> "..out_passw, in_entropy
end