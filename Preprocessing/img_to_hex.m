clc;

% Folder containing images
folder = '9';   % change this to your folder name
files = dir(fullfile(folder, '*.jpg'));  % take all .jpg files

% Open output file once
fid = fopen('1009test.hex', 'wt');

for idx = 1:length(files)
    % Get image path
    img_path = fullfile(folder, files(idx).name);

    % Read input image
    b = imread(img_path);

    % Convert to grayscale if RGB
    if size(b, 3) == 3
        b = rgb2gray(b);
    end

    % Resize to 28x28
    b = imresize(b, [28 28]);

    % Convert to 1D array (top-left to bottom-right)
    k = 1;
    for i = 28:-1:1
        for j = 28:-1:1
            a(k) = b(i, j);
            if a(k) < 75
                a(k) = 0;
            else
                a(k) = 1;
            end
            k = k + 1;
        end
    end

    % Write pixel data for this image
    fprintf(fid, '%01x', a);

    % Newline after each image
    fprintf(fid, '\n');
end

disp('All images written to file');
fclose(fid);
