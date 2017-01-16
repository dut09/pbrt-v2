writerObj = VideoWriter('j15_bridge3_uni_64.avi');
writerObj.FrameRate = 30;
% open the video writer
open(writerObj);
% write the frames to the video
img_num = (length(dir('rendering')) - 2) / 2;
for u = 0 : (img_num - 1)
 % convert the image to a frame
 img = imread(['rendering/j15_bridge3_uni_64_', num2str(u, '%03d'), '.tiff']);
 frame = im2frame(img(:, :, 1 : 3));
 writeVideo(writerObj, frame);
end
% close the writer object
close(writerObj);