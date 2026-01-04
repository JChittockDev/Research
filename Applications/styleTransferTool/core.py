import pystiche
from pystiche import demo, enc, loss, optim
from pystiche.image import show_image
from pystiche.misc import get_device, get_input_image
from pystiche.image import read_image

class styleTransfer():
    
    def get_style_op(self, encoder, layer_weight):
        return loss.GramLoss(encoder, score_weight=layer_weight)
        
    def transferStyle(self, content_weight, style_weight, reduce, reduced_size, steps, content_path, style_path):
        device = get_device()
        multi_layer_encoder = enc.vgg19_multi_layer_encoder()
        content_layer = "relu4_2"
        content_encoder = multi_layer_encoder.extract_encoder(content_layer)
        content_loss = loss.FeatureReconstructionLoss(content_encoder, score_weight=content_weight)
        style_layers = ("relu1_1", "relu2_1", "relu3_1", "relu4_1", "relu5_1")
        style_loss = loss.MultiLayerEncodingLoss(multi_layer_encoder, style_layers, self.get_style_op, score_weight=style_weight)
        perceptual_loss = loss.PerceptualLoss(content_loss, style_loss).to(device)
        
        if reduce:
            content_image = read_image(content_path, size=reduced_size, device=device)
            style_image = read_image(style_path, size=reduced_size, device=device)
        else:
            content_image = read_image(content_path, device=device)
            style_image = read_image(style_path, device=device)

        perceptual_loss.set_content_image(content_image)
        perceptual_loss.set_style_image(style_image)

        starting_point = "content"
        input_image = get_input_image(starting_point, content_image=content_image)
        output_image = optim.image_optimization(input_image, perceptual_loss, num_steps=steps)
        show_image(output_image)