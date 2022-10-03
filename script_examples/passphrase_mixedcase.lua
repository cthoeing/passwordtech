-- Example Lua script for Password Tech
-- Convert passphrase letters into mixed-case letters, meaning that
-- each letter is converted to upper-case or lower-case with a 50:50 chance.

function generate(passw_num, in_passw, in_entropy)
	if not in_passw then
		return "empty password", 0
	end
	local out_passw=""
	local out_entropy=in_entropy
	for i=1,#in_passw do
		local c=string.byte(in_passw,i)
		local mode=0
		-- if character is a letter, detect case
		if c>=65 and c<=90 then
		    mode=1 --uppercase
		elseif c>=97 and c<=122 then
		    mode=2 --lowercase
		end
		-- convert upper-case letter to lower-case
		-- and vice versa, with a 50:50 chance
		if mode>0 then
			if pwtech_random(2)==1 then
				if mode==1 then
					c=97+c-65
				else
					c=65+c-97
				end
			end
			-- increase entropy by 1, assuming that the
			-- input password is not mixed-case
			out_entropy=out_entropy+1
		end
		out_passw=out_passw..string.char(c)
	end
	return out_passw, out_entropy
end