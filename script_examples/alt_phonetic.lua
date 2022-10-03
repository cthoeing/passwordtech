-- Example Lua script for Password Tech
-- Generate phonetic password based on very simple rules

script_flags=1

-- vocals, consonants, and entropy values
vowels = "aeiou"
consonants = "bcdfghjklmnpqrstvwxyz"
ent_v = math.log(#vowels)/math.log(2)
ent_c = math.log(#consonants)/math.log(2)

-- desired number of characters
num=0

function init(num_passw, dest, gen_flags, advanced_flags, num_chars, num_words, format_str)
	-- if number of characters is 0, use default value of 8
	num=num_chars==0 and 8 or num_chars
end

function generate(passw_num, in_passw, in_entropy)
	local out_passw="" -- resulting password
	local out_entropy=0 -- resulting entropy
	local nv, nc = 1, 1 -- number of consecutive vocals and consonants
	local c
	for i=1,num do
		-- enforce vowel/consonant in case of 2 consecutive consonants/vowels;
		-- select randomly otherwise
		if nc>1 or (nv<2 and pwtech_random(2)==1) then
		    -- choose random vowel from string
			r=pwtech_random(#vowels)
			c=string.sub(vowels,r,r)
			out_entropy=out_entropy+ent_v
			nv=nv+1
			nc=0
		else
		    -- choose random consonant from string
			r=pwtech_random(#consonants)
			c=string.sub(consonants,r,r)
			out_entropy=out_entropy+ent_c
			nc=nc+1
			nv=0
		end
		-- append character to password
		out_passw=out_passw..c
	end
	return out_passw, out_entropy
end