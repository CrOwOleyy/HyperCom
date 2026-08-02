#pragma once

namespace hypercom::client {

// Apparition « en bulle » d'un bloc d'interface.
//
// ImGui ne sait pas mettre un widget a l'echelle apres coup : on ne peut donc
// pas vraiment faire grossir un panneau. L'illusion tient a trois choses jouees
// ensemble -- l'opacite qui monte, le bloc qui remonte depuis le bas, et un
// leger depassement en fin de course qui donne le rebond d'une bulle qui creve
// la surface.

[[nodiscard]] float ease_out_cubic(float progress);

// Depasse la cible avant d'y revenir. C'est ce depassement qui fait « bulle »
// plutot que « fondu ».
[[nodiscard]] float ease_out_back(float progress);

// A appairer systematiquement avec end_bubble_reveal, y compris si le bloc
// dessine entre les deux fait un retour anticipe.
void begin_bubble_reveal(float progress, float scale);

void end_bubble_reveal();

} // namespace hypercom::client
