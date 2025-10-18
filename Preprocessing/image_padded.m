% Input and output filenames
infile = 'mnist_10ktest.hex';   % your input 10000-line, 784-bit file
outfile = 'mnist_10ktestpadded.hex'; % resulting 20000-line, 512-bit file

% Read all lines
fid = fopen(infile, 'r');
lines = textscan(fid, '%s');
fclose(fid);
lines = lines{1};

n = numel(lines);
out = cell(2*n,1);

for i = 1:n
    x = lines{i};
    
    % Ensure each line has exactly 784 bits
    if length(x) < 784
        x = [repmat('0',1,784-length(x)) x];
    elseif length(x) > 784
        x = x(end-783:end);
    end
    
    % LSB 512 bits (lower part)
    lsb = x(end-511:end);
    
    % Remaining 272 bits (MSB part)
    msb = x(1:272);
    
    % Zero-pad MSB line to 512 bits
    msb_padded = [repmat('0',1,512-272) msb];
    
    % Store
    out{2*i-1} = lsb;
    out{2*i} = msb_padded;
end

% Write output file
fid = fopen(outfile, 'w');
for i = 1:numel(out)
    fprintf(fid, '%s\n', out{i});
end
fclose(fid);

disp('Done. Generated 20000 lines of 512 bits each.');
