import matplotlib.pyplot as plt
import ast

try:
    plt.rcParams['keymap.save'].remove('s')
except (ValueError, KeyError):
    pass


class SegmentEditor:
    def __init__(self):
        self.fig, self.ax = plt.subplots(figsize=(8, 8))

        self.segments = []

        self.current_start = None

        self.line, = self.ax.plot([], [], 'o-', color='blue', markersize=4)

        self.preview_line, = self.ax.plot(
            [], [], '--', color='red', linewidth=0.5)

        self.start_point_marker, = self.ax.plot(
            [], [], 'o', color='green', markersize=6)

        self.ax.set_title(
            "Edytor Odcinków\n1. Klik: Start | 2. Klik: Koniec | 's': Zapisz | 'z': Cofnij")
        self.ax.set_xlim(0, 10)
        self.ax.set_ylim(0, 10)
        self.ax.grid(True)

        self.cid_click = self.fig.canvas.mpl_connect(
            'button_press_event', self.on_click)
        self.cid_key = self.fig.canvas.mpl_connect(
            'key_press_event', self.on_key)
        self.cid_move = self.fig.canvas.mpl_connect(
            'motion_notify_event', self.on_move)

        print("=== INSTRUKCJA ===")
        print("Rysowanie odcinków:")
        print("  - Pierwsze kliknięcie [LPM]: Wybierz początek odcinka.")
        print("  - Drugie kliknięcie [LPM]: Wybierz koniec odcinka.")
        print("[PPM] - Anuluj aktualnie zaczęty odcinek (jeśli jest)")
        print("[s]   - Zapisz współrzędne odcinków do pliku 'segments_data.txt'")
        print("[l]   - Wczytaj z pliku 'segments_data.txt'")
        print("[z]   - Cofnij (usuwa ostatni pełny odcinek)")
        print("[r]   - Resetuj (wyczyść wszystko)")

        plt.show()

    def update_plot(self):
        xs = []
        ys = []

        for p1, p2 in self.segments:
            xs.extend([p1[0], p2[0], None])
            ys.extend([p1[1], p2[1], None])

        self.line.set_data(xs, ys)
        self.fig.canvas.draw()

    def on_click(self, event):
        if event.inaxes != self.ax or self.fig.canvas.toolbar.mode != '':
            return

        # Lewy Przycisk Myszy (LPM)
        if event.button == 1:
            x, y = round(event.xdata, 2), round(event.ydata, 2)

            if self.current_start is None:
                self.current_start = (x, y)
                self.start_point_marker.set_data([x], [y])
                print(f"Start odcinka: {self.current_start}")
            else:
                start = self.current_start
                end = (x, y)
                self.segments.append((start, end))

                print(f"Koniec odcinka: {end}. Zapisano odcinek.")
                # Resetujemy stan rysowania
                self.current_start = None
                self.start_point_marker.set_data([], [])
                self.preview_line.set_data([], [])
                self.update_plot()

        # Prawy Przycisk Myszy (PPM) - Anulowanie rozpoczęcia
        elif event.button == 3:
            if self.current_start is not None:
                self.current_start = None
                self.start_point_marker.set_data([], [])
                self.preview_line.set_data([], [])
                self.fig.canvas.draw()
                print("Anulowano rysowanie obecnego odcinka.")

    def on_move(self, event):
        if event.inaxes != self.ax:
            return
        if self.current_start:
            self.preview_line.set_data(
                [self.current_start[0], event.xdata],
                [self.current_start[1], event.ydata]
            )
            self.fig.canvas.draw()

    def on_key(self, event):
        if event.key == 's':
            # Zapisujemy listę par punktów
            with open("segments_data.txt", "w") as f:
                f.write(str(self.segments))
            print(
                f"\nZapisano {len(self.segments)} odcinków do pliku segments_data.txt")
            self.ax.set_title("Zapisano!")
            self.fig.canvas.draw()

        elif event.key == 'l':
            try:
                with open("segments_data.txt", "r") as f:
                    content = f.read()
                    loaded_data = ast.literal_eval(content)
                    if isinstance(loaded_data, list):
                        self.current_start = None
                        self.start_point_marker.set_data([], [])
                        self.update_plot()
                        print(f"\nWczytano {len(self.segments)} odcinków.")
                        self.ax.set_title("Wczytano z pliku!")
            except FileNotFoundError:
                print("Nie znaleziono pliku segments_data.txt")
            except Exception as e:
                print(f"Błąd podczas wczytywania: {e}")

        elif event.key == 'z':
            # Cofnij: usuwa ostatni pełny odcinek
            if self.segments:
                removed = self.segments.pop()
                print(f"Usunięto odcinek: {removed}")
                self.update_plot()
            elif self.current_start:
                self.current_start = None
                self.start_point_marker.set_data([], [])
                self.preview_line.set_data([], [])
                self.fig.canvas.draw()
                print("Cofnięto punkt startowy.")

        elif event.key == 'r':
            self.segments = []
            self.current_start = None
            self.start_point_marker.set_data([], [])
            self.preview_line.set_data([], [])
            self.update_plot()
            print("Wyczyszczono.")
            self.ax.set_title("Edytor Odcinków")
            self.fig.canvas.draw()


if __name__ == "__main__":
    editor = SegmentEditor()
