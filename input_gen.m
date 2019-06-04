
size = 64;
x = linspace(0, 2*pi, size);
input = sin(x);
output = fft(input);
in_real = real(input);
in_imag = imag(input);
out_real = real(output);
out_imag = imag(output);


in_real_array = vec_to_c_array(in_real);
in_imag_array = vec_to_c_array(in_imag);
out_real_array =  vec_to_c_array(out_real);
out_imag_array = vec_to_c_array(out_imag);
magnitude_array = vec_to_c_array(abs(output));


disp(sprintf("float input_real[%d] = ", size) + in_real_array + ";");
disp(sprintf("float input_imag[%d] = ", size) + in_imag_array + ";");
disp(sprintf("float expected_output_real[%d] = ", size) + out_real_array + ";");
disp(sprintf("float expected_output_imag[%d] = ", size) + out_imag_array + ";");
disp(sprintf("float expected_magnitude[%d] = ", size) + magnitude_array + ";");