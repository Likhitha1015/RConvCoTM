% File paths
inputFile = 'MNIST_7_1_clauses.txt';      % your input text file
outputFile = 'MNIST_clauses_7_1_formatted.txt';    % output file to save padded lines

% Read all lines
fid = fopen(inputFile, 'r');
lines = textscan(fid, '%s');
fclose(fid);
lines = lines{1};

% Process each line
numLines = numel(lines);
paddedLines = cell(numLines, 1);

for i = 1:numLines
    binStr = strtrim(lines{i});  % remove any extra spaces/newlines
    len = length(binStr);
    if len > 256
        warning('Line %d has more than 256 bits (%d). Truncating.', i, len);
        binStr = binStr(end-255:end);  % keep lower 256 bits
    else
        padLen = 256 - len;
        binStr = [repmat('0', 1, padLen) binStr]; % pad zeros from MSB
    end
    paddedLines{i} = binStr;
end

% Write padded lines to file
fid = fopen(outputFile, 'w');
for i = 1:numLines
    fprintf(fid, '%s\n', paddedLines{i});
end
fclose(fid);

disp('✅ Padding complete. Output saved to output.txt');
