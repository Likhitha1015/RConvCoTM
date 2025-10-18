% === Input and Output File Paths ===
f4  = 'KMNIST_weights_3_1.txt';  % Input file (with 0b prefixes)
f5a = 'KMNIST_3_1_weights[0].txt';
f5b = 'KMNIST_3_1_weights[1].txt';
f5c = 'KMNIST_3_1_weights[2].txt';

% === Read All Lines from Input ===
fid = fopen(f4, 'r');
lines = textscan(fid, '%s', 'Delimiter', '\n', 'Whitespace', '');
fclose(fid);
lines = strtrim(lines{1});

% === Remove '0b' Prefix ===
lines = regexprep(lines, '^0b', '');

% === Concatenate Every 140 Lines into One Line ===
groupSize = 140;
numLines = numel(lines);
numGroups = ceil(numLines / groupSize);
concatLines = cell(numGroups, 1);

for g = 1:numGroups
    startIdx = (g-1)*groupSize + 1;
    endIdx = min(g*groupSize, numLines);
    groupLines = lines(startIdx:endIdx);
    concatLines{g} = strjoin(groupLines, '');  % join without spaces
end

fprintf('✅ Concatenated into %d lines (each = 140 lines combined)\n', numGroups);

% === Split Each Line into 3 Parts (Like Your Logic) ===
fid1 = fopen(f5a, 'w');
fid2 = fopen(f5b, 'w');
fid3 = fopen(f5c, 'w');

for i = 1:length(concatLines)
    s = concatLines{i};
    % Pad to 1260 bits (if shorter)
    if numel(s) < 1260
        s = pad(s, 1260, 'right', '0');
    end
    
    % Extract 3 slices
    p1 = s(1260-511  : 1260-0);    % bits 511–0
    p2 = s(1260-1023 : 1260-512);  % bits 1023–512
    p3 = s(1260-1259 : 1260-1024); % bits 1259–1024
    
    % Write to files
    fprintf(fid1, '%s\n', p1);
    fprintf(fid2, '%s\n', p2);
    fprintf(fid3, '%s\n', p3);
end

fclose(fid1);
fclose(fid2);
fclose(fid3);

fprintf('✅ Wrote final split files:\n   %s\n   %s\n   %s\n', f5a, f5b, f5c);
fprintf('✔️ All steps complete: %d concatenated blocks processed\n', numGroups);
