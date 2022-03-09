#include <stdio.h>
#include <gtk\gtk.h>
#include <unistd.h>
#include <hb_save.h>
// #include <windows.h>

static void
activate (GtkApplication* app,
          gpointer        user_data)
{
  GtkWidget *window;

  window = gtk_application_window_new (app);
  gtk_window_set_title (GTK_WINDOW (window), "Window");
  gtk_window_set_default_size (GTK_WINDOW (window), 200, 200);
  gtk_widget_show (window);
}

int
main (int    argc,
      char **argv)
{
  chdir("C:\\Users\\Tiago\\Desktop\\C\\Projetos\\Heartbound Save Editor\\release");
  // LONGLONG r = LONG_LONG_MAX;
  // for (size_t i = 0; i < 10000; i++)
  // {
  //   LARGE_INTEGER start, end, freq;
  //   QueryPerformanceFrequency(&freq);
  //   QueryPerformanceCounter(&start);
    open_save();
    // QueryPerformanceCounter(&end);
    // LONGLONG total = end.QuadPart - start.QuadPart;
    // r = (total < r) ? total : r;
    close_save();
  // }
  // printf("%d", r);

  GtkApplication *app;
  int status;

  app = gtk_application_new ("org.gtk.example", G_APPLICATION_FLAGS_NONE);
  g_signal_connect (app, "activate", G_CALLBACK (activate), NULL);
  status = g_application_run (G_APPLICATION (app), argc, argv);
  g_object_unref (app);

  return status;
}