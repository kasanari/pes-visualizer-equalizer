function array = vec_to_c_array(vector)

array = "{";
for i = 1:(length(vector)-1)
    array = array + sprintf("%f, ", vector(i));
end
array = array + sprintf("%f}", vector(end));
end

