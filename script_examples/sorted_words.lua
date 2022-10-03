-- Example Lua script for Password Tech
-- Generate passphrase with words arranged in sorted order (longest word first)
-- Number of words is to be specified via the "Number" field in the main window

script_flags=1
N=0
entropy=math.log(2)

function init(num_passw, dest, gen_flags, advanced_flags, num_chars, num_words, format_str)
	N=num_words
	entropy=N*math.log(pwtech_numwords())/math.log(2)
end

function generate(passw_num, in_passw, in_entropy)
	if N>0 then
	    -- initialize table with N random words
		local words={}
		for i=1,N do
			words[i]=pwtech_word()
		end
		
		-- sort table by word lengths in descending order
		-- (longest word first)
		table.sort(words, function(a,b) return #a>#b end)
		
		-- concatenate words to get passphrase
		local out_passw=""
		for i=1,N do
			out_passw=out_passw..words[i]
			if i<N then out_passw=out_passw.." " end
		end
		
		-- return password and calculated entropy
		return out_passw, entropy
	end
	return "number of words must be >0", 0
end