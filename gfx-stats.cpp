#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <cctype>
#include <cassert>
#include <cstdint>

#ifdef linux
#undef linux // incredibly confusing bugs happened as this is defined to 1
#endif

enum class OS {
  unknown = 0,
  win81,
  win8,
  win7,
  winvista,
  winxp,
  win_other,
  mac,
  gnulinux,
  android,
  count
};

OS OS_begin()
{
  return OS::unknown;
}

OS OS_end()
{
  return OS::count;
}

OS OS_next(OS os)
{
  assert(os != OS_end());
  return OS(int(os) + 1);
}

uint32_t bit(OS os)
{
  assert(os != OS_end());
  return uint32_t(1) << int(os);
}

enum class GPUVendor {
  unknown = 0,
  intel,
  nvidia,
  amd,
  arm,
  qualcomm,
  imgtec,
  vivante,
  broadcom,
  hisilicon,
  sis,
  s3,
  via,
  advancedgraphics,
  count
};

GPUVendor GPUVendor_begin()
{
  return GPUVendor::unknown;
}

GPUVendor GPUVendor_end()
{
  return GPUVendor::count;
}

GPUVendor GPUVendor_next(GPUVendor os)
{
  assert(os != GPUVendor_end());
  return GPUVendor(int(os) + 1);
}

uint32_t bit(GPUVendor v)
{
  assert(v != GPUVendor_end());
  return uint32_t(1) << int(v);
}

template<typename IndexType,
         IndexType SizeAsEnumValue,
         typename ValueType>
class EnumeratedArray
{
public:
  static const size_t kSize = size_t(SizeAsEnumValue);

private:
  ValueType mArray[kSize];

public:
  EnumeratedArray()
  {
    memset(mArray, 0, sizeof(mArray));
  }

  ValueType& operator[](IndexType aIndex)
  {
    return mArray[size_t(aIndex)];
  }

  const ValueType& operator[](IndexType aIndex) const
  {
    return mArray[size_t(aIndex)];
  }
};

OS os_for_line(const char *line)
{
  if (const char *occurence = strstr(line, "Windows NT"))
  {
    if (strstr(occurence, "Windows NT\t6.3") == occurence)
      return OS::win81;
    else if (strstr(occurence, "Windows NT\t6.2") == occurence)
      return OS::win8;
    else if (strstr(occurence, "Windows NT\t6.1") == occurence)
      return OS::win7;
    else if (strstr(occurence, "Windows NT\t6.0") == occurence)
      return OS::winvista;
    else if (strstr(occurence, "Windows NT\t5.1") == occurence)
      return OS::winxp;
    else
      return OS::win_other;
  }
  else if (strstr(line, "Mac OS X"))
  {
    return OS::mac;
  }
  else if (strstr(line, "Linux"))
  {
    if (strstr(line, "Android"))
      return OS::android;
    else
      return OS::gnulinux;
  }
  return OS::unknown;
}

GPUVendor desktop_gpuvendor_for_line(const char *line)
{
  const char *prefix = "AdapterVendorID: ";
  const char *where = strstr(line, prefix);
  if (!where) {
    return GPUVendor::unknown;
  }
  where += strlen(prefix);

  if (where == strstr(where, "0x8086") || where == strstr(where, "8086")) {
    return GPUVendor::intel;
  }
  if (where == strstr(where, "0x10de") || where == strstr(where, "10de")) {
    return GPUVendor::nvidia;
  }
  if (where == strstr(where, "0x1002") || where == strstr(where, "1002")) {
    return GPUVendor::amd;
  }
  if (where == strstr(where, "0x1039") || where == strstr(where, "1039")) {
    return GPUVendor::sis;
  }
  if (where == strstr(where, "0x5333") || where == strstr(where, "5333")) {
    return GPUVendor::s3;
  }
  if (where == strstr(where, "0x1106") || where == strstr(where, "1106")) {
    return GPUVendor::via;
  }
  return GPUVendor::unknown;
}

GPUVendor android_gpuvendor_for_line(const char *line)
{
  const char *prefix = "OpenGL: ";
  const char *where = strstr(line, prefix);
  if (!where) {
    prefix = "AdapterDescription: '";
    where = strstr(line, prefix);
  }
  if (!where) {
    return GPUVendor::unknown;
  }
  where += strlen(prefix);

  if (where == strstr(where, "Qualcomm --")) {
    return GPUVendor::qualcomm;
  }
  if (where == strstr(where, "ARM --")) {
    return GPUVendor::arm;
  }
  if (where == strstr(where, "Imagination Technologies --") || where == strstr(where, "PowerVR --")) {
    return GPUVendor::imgtec;
  }
  if (where == strstr(where, "NVIDIA Corporation --")) {
    return GPUVendor::nvidia;
  }
  if (where == strstr(where, "Vivante Corporation --") || where == strstr(where, "Marvell Technology Group Ltd --")) {
    return GPUVendor::vivante;
  }
  if (where == strstr(where, "Broadcom --")) {
    return GPUVendor::broadcom;
  }
  if (where == strstr(where, "Hisilicon Technologies --")) {
    return GPUVendor::hisilicon;
  }
  if (where == strstr(where, "Intel Open Source Technology Center --")) {
    return GPUVendor::intel;
  }
  if (where == strstr(where, "Advanced Micro Devices, Inc. --")) {
    return GPUVendor::amd;
  }
  if (where == strstr(where, "Advanced Graphics Corporation --")) {
    return GPUVendor::advancedgraphics;
  }
  return GPUVendor::unknown;
}

struct feature
{
  const char * const name;
  const size_t name_length;
  const feature& featureCountingAttempts;
  EnumeratedArray<OS, OS::count, int>
    attempts,
    successes,
    reports;

  feature(const char *n)
    : name(n), name_length(strlen(name)), featureCountingAttempts(*this)
  {
  }

  feature(const char *n, const feature& f)
    : name(n), name_length(strlen(name)), featureCountingAttempts(f)
  {
  }

  void process(const char *line, OS os)
  {
    bool is_release_channel = strstr(line, "\trelease\t");
    int weight = (is_release_channel && os != OS::android) ? 10 : 1;

    reports[os] += weight;
    const char *ptr = line;
    bool attempted = false;
    bool succeeded = false;
    while((ptr = strstr(ptr, name)))
    {
      ptr += name_length;
      if (*ptr == '?' || *ptr == '!')
        attempted = true;
      else if (*ptr == '+')
        succeeded = true;
    }
    if (attempted)
      attempts[os] += weight;
    if (succeeded)
    {
      successes[os] += weight;
    }
  }

  float success_percentage(uint32_t os_bitfield)
  {
    int combined_successes = 0, combined_attempts = 0;
    for (OS os = OS_begin(); os != OS_end(); os = OS_next(os))
    {
      if (os_bitfield & bit(os))
      {
        combined_successes += successes[os];
        combined_attempts += featureCountingAttempts.attempts[os];
      }
    }
    return combined_attempts ? 100.f * combined_successes / combined_attempts
                             : 0.f;
  }

  float attempt_percentage(uint32_t os_bitfield)
  {
    assert(&featureCountingAttempts == this);
    int combined_reports = 0, combined_attempts = 0;
    for (OS os = OS_begin(); os != OS_end(); os = OS_next(os))
    {
      if (os_bitfield & bit(os))
      {
        combined_reports += reports[os];
        combined_attempts += attempts[os];
      }
    }
    return combined_reports ? 100.f * combined_attempts / combined_reports
                            : 0.0f;
  }

  float os_market_share(uint32_t os_bitfield)
  {
    int combined_reports_these_oses = 0, combined_reports_all_oses = 0;
    for (OS os = OS_begin(); os != OS_end(); os = OS_next(os))
    {
      combined_reports_all_oses += reports[os];
      if (os_bitfield & bit(os))
      {
        combined_reports_these_oses += reports[os];
      }
    }
    return combined_reports_all_oses ? 100.f * combined_reports_these_oses / combined_reports_all_oses
                                     : 0.f;
  }

  int num_reports(uint32_t os_bitfield)
  {
    int combined_reports_these_oses = 0;
    for (OS os = OS_begin(); os != OS_end(); os = OS_next(os))
    {
      if (os_bitfield & bit(os))
      {
        combined_reports_these_oses += reports[os];
      }
    }
    return combined_reports_these_oses;
  }

};

feature webgl("WebGL");
feature layers("Layers");
feature d3d10layers("D3D10 Layers", layers);
feature d3d11layers("D3D11 Layers", layers);

EnumeratedArray<GPUVendor, GPUVendor::count, int>
  desktop_gpuvendor_reports,
  android_gpuvendor_reports;

void process_gpuvendors(const char *line, OS os)
{
  switch (os) {
    case OS::win81:
    case OS::win8:
    case OS::win7:
    case OS::winvista:
    case OS::winxp:
    case OS::win_other:
    case OS::mac:
    // gnulinux uses a different format, just omit it.
    {
      GPUVendor v = desktop_gpuvendor_for_line(line);
      desktop_gpuvendor_reports[v]++;
      break;
    }
    case OS::android: {
      GPUVendor v = android_gpuvendor_for_line(line);
      android_gpuvendor_reports[v]++;
      break;
    }
    default:
      break;
  }
}

float desktop_gpuvendor_market_share(GPUVendor gpuvendor)
{
  int total = 0;
  for (GPUVendor v = GPUVendor_begin(); v != GPUVendor_end(); v = GPUVendor_next(v)) {
    if (v != GPUVendor::unknown) {
      total += desktop_gpuvendor_reports[v];
    }
  }
  if (total == 0) return 0.f;
  return 100 * float(desktop_gpuvendor_reports[gpuvendor]) / total;
}

float android_gpuvendor_market_share(GPUVendor gpuvendor)
{
  int total = 0;
  for (GPUVendor v = GPUVendor_begin(); v != GPUVendor_end(); v = GPUVendor_next(v)) {
    if (v != GPUVendor::unknown) {
      total += android_gpuvendor_reports[v];
    }
  }
  if (total == 0) return 0.f;
  return 100 * float(android_gpuvendor_reports[gpuvendor]) / total;
}

bool enable_layers_stats_for_os_bitfield(uint32_t os_bitfield)
{
  return os_bitfield & ~bit(OS::gnulinux);
}

bool enable_d3d10layers_stats_for_os_bitfield(uint32_t os_bitfield)
{
  uint32_t mask = bit(OS::winvista) |
                  bit(OS::win7) |
                  bit(OS::win8) |
                  bit(OS::win81);
  return os_bitfield == (os_bitfield & mask);
}

bool enable_d3d11layers_stats_for_os_bitfield(uint32_t os_bitfield)
{
  return enable_d3d10layers_stats_for_os_bitfield(os_bitfield);
}

void output(const char *date, uint32_t os_bitfield, const char *filename)
{
  static const size_t max_output_line_size = 256;
  char wanted_first_line[max_output_line_size];
  snprintf(wanted_first_line, max_output_line_size, "date,%s", webgl.name);

  if (enable_layers_stats_for_os_bitfield(os_bitfield))
  {
    strcat(wanted_first_line, ",");
    strcat(wanted_first_line, layers.name);
  }
  if (enable_d3d10layers_stats_for_os_bitfield(os_bitfield))
  {
    strcat(wanted_first_line, ",");
    strcat(wanted_first_line, d3d10layers.name);
  }
  if (enable_d3d11layers_stats_for_os_bitfield(os_bitfield))
  {
    strcat(wanted_first_line, ",");
    strcat(wanted_first_line, d3d11layers.name);
  }
  strcat(wanted_first_line, "\n");

  char line[max_output_line_size];

  FILE *rofile = fopen(filename, "r");

  bool should_write_first_line = true;
  if (rofile && fgets(line, max_output_line_size, rofile))
  {
    if (strcmp(line, wanted_first_line))
    {
      fprintf(stderr, "error: first line of %s is:\n%sexpected:\n%sNot appending data.\n", filename, line, wanted_first_line);
      return;
    }
    should_write_first_line = false;
  }

  fclose(rofile);

  FILE *file = fopen(filename, "a");

  if (!file)
  {
    fprintf(stderr, "Can't open %s for appending and reading\n", filename);
    exit(1);
  }

  if (should_write_first_line) {
    fprintf(file, "%s", wanted_first_line);
  }

  fprintf(file, "%s,%.2f", date, webgl.success_percentage(os_bitfield));
  if (enable_layers_stats_for_os_bitfield(os_bitfield))
    fprintf(file, ",%.2f", layers.success_percentage(os_bitfield));
  if (enable_d3d10layers_stats_for_os_bitfield(os_bitfield))
    fprintf(file, ",%.2f", d3d10layers.success_percentage(os_bitfield));
  if (enable_d3d11layers_stats_for_os_bitfield(os_bitfield))
    fprintf(file, ",%.2f", d3d11layers.success_percentage(os_bitfield));
  fprintf(file, "\n");

  fclose(file);
}

void output_webgl_attempt_percentage(const char *date, uint32_t os_bitfield, const char *filename)
{
  static const size_t max_output_line_size = 256;
  char wanted_first_line[max_output_line_size];
  snprintf(wanted_first_line, max_output_line_size,
           "date,WebGL attempt rate\n");

  char line[max_output_line_size];

  FILE *rofile = fopen(filename, "r");

  bool should_write_first_line = true;
  if (rofile && fgets(line, max_output_line_size, rofile))
  {
    if (strcmp(line, wanted_first_line))
    {
      fprintf(stderr, "error: first line of %s is:\n%sexpected:\n%sNot appending data.\n", filename, line, wanted_first_line);
      return;
    }
    should_write_first_line = false;
  }

  fclose(rofile);

  FILE *file = fopen(filename, "a");

  if (!file)
  {
    fprintf(stderr, "Can't open %s for appending and reading\n", filename);
    exit(1);
  }

  if (should_write_first_line) {
    fprintf(file, "%s", wanted_first_line);
  }

  fprintf(file, "%s,%.2f\n", date, webgl.attempt_percentage(os_bitfield));

  fclose(file);
}

void output_os_market_share(const char *date, const char *filename)
{
  static const size_t max_output_line_size = 256;
  char wanted_first_line[max_output_line_size];
  snprintf(wanted_first_line, max_output_line_size,
           "date,Windows 8.1,Windows 8.0,Windows 7,Windows Vista,Windows XP,Mac OSX,Linux,Android\n");

  char line[max_output_line_size];

  FILE *rofile = fopen(filename, "r");

  bool should_write_first_line = true;
  if (rofile && fgets(line, max_output_line_size, rofile))
  {
    if (strcmp(line, wanted_first_line))
    {
      fprintf(stderr, "error: first line of %s is:\n%sexpected:\n%sNot appending data.\n", filename, line, wanted_first_line);
      return;
    }
    should_write_first_line = false;
  }

  fclose(rofile);

  FILE *file = fopen(filename, "a");

  if (!file)
  {
    fprintf(stderr, "Can't open %s for appending and reading\n", filename);
    exit(1);
  }

  if (should_write_first_line) {
    fprintf(file, "%s", wanted_first_line);
  }

  fprintf(file, "%s,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f\n",
          date,
          webgl.os_market_share(bit(OS::win81)),
          webgl.os_market_share(bit(OS::win8)),
          webgl.os_market_share(bit(OS::win7)),
          webgl.os_market_share(bit(OS::winvista)),
          webgl.os_market_share(bit(OS::winxp)),
          webgl.os_market_share(bit(OS::mac)),
          webgl.os_market_share(bit(OS::gnulinux)),
          webgl.os_market_share(bit(OS::android))
         );

  fclose(file);
}

void output_desktop_gpuvendor_market_share(const char *date, const char *filename)
{
  static const size_t max_output_line_size = 256;
  char wanted_first_line[max_output_line_size];
  snprintf(wanted_first_line, max_output_line_size,
           "date,Intel,NVIDIA,AMD (ATI),SIS,S3,VIA\n");

  char line[max_output_line_size];

  FILE *rofile = fopen(filename, "r");

  bool should_write_first_line = true;
  if (rofile && fgets(line, max_output_line_size, rofile))
  {
    if (strcmp(line, wanted_first_line))
    {
      fprintf(stderr, "error: first line of %s is:\n%sexpected:\n%sNot appending data.\n", filename, line, wanted_first_line);
      return;
    }
    should_write_first_line = false;
  }

  fclose(rofile);

  FILE *file = fopen(filename, "a");

  if (!file)
  {
    fprintf(stderr, "Can't open %s for appending and reading\n", filename);
    exit(1);
  }

  if (should_write_first_line) {
    fprintf(file, "%s", wanted_first_line);
  }

  fprintf(file, "%s,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f\n",
          date,
          desktop_gpuvendor_market_share(GPUVendor::intel),
          desktop_gpuvendor_market_share(GPUVendor::nvidia),
          desktop_gpuvendor_market_share(GPUVendor::amd),
          desktop_gpuvendor_market_share(GPUVendor::sis),
          desktop_gpuvendor_market_share(GPUVendor::s3),
          desktop_gpuvendor_market_share(GPUVendor::via)
         );

  fclose(file);
}

void output_android_gpuvendor_market_share(const char *date, const char *filename)
{
  static const size_t max_output_line_size = 256;
  char wanted_first_line[max_output_line_size];
  snprintf(wanted_first_line, max_output_line_size,
           "date,Qualcomm (Adreno),ARM (Mali),NVIDIA (Tegra),ImgTec (PowerVR),Vivante (also Marvell), Broadcom (VideoCore), Hisilicon, Intel, AMD, Advanced Graphics\n");

  char line[max_output_line_size];

  FILE *rofile = fopen(filename, "r");

  bool should_write_first_line = true;
  if (rofile && fgets(line, max_output_line_size, rofile))
  {
    if (strcmp(line, wanted_first_line))
    {
      fprintf(stderr, "error: first line of %s is:\n%sexpected:\n%sNot appending data.\n", filename, line, wanted_first_line);
      return;
    }
    should_write_first_line = false;
  }

  fclose(rofile);

  FILE *file = fopen(filename, "a");

  if (!file)
  {
    fprintf(stderr, "Can't open %s for appending and reading\n", filename);
    exit(1);
  }

  if (should_write_first_line) {
    fprintf(file, "%s", wanted_first_line);
  }

  fprintf(file, "%s,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f\n",
          date,
          android_gpuvendor_market_share(GPUVendor::qualcomm),
          android_gpuvendor_market_share(GPUVendor::arm),
          android_gpuvendor_market_share(GPUVendor::nvidia),
          android_gpuvendor_market_share(GPUVendor::imgtec),
          android_gpuvendor_market_share(GPUVendor::vivante),
          android_gpuvendor_market_share(GPUVendor::broadcom),
          android_gpuvendor_market_share(GPUVendor::hisilicon),
          android_gpuvendor_market_share(GPUVendor::intel),
          android_gpuvendor_market_share(GPUVendor::amd),
          android_gpuvendor_market_share(GPUVendor::advancedgraphics)
         );

  fclose(file);
}

void output_num_reports(const char *date, const char *filename)
{
  static const size_t max_output_line_size = 256;
  char wanted_first_line[max_output_line_size];
  snprintf(wanted_first_line, max_output_line_size,
          "date,Number of reports\n");

  char line[max_output_line_size];

  FILE *rofile = fopen(filename, "r");

  bool should_write_first_line = true;
  if (rofile && fgets(line, max_output_line_size, rofile))
  {
    if (strcmp(line, wanted_first_line))
    {
      fprintf(stderr, "error: first line of %s is:\n%sexpected:\n%sNot appending data.\n", filename, line, wanted_first_line);
      return;
    }
    should_write_first_line = false;
  }

  fclose(rofile);

  FILE *file = fopen(filename, "a");

  if (!file)
  {
    fprintf(stderr, "Can't open %s for appending and reading\n", filename);
    exit(1);
  }

  if (should_write_first_line) {
    fprintf(file, "%s", wanted_first_line);
  }

  fprintf(file, "%s,%d\n",
          date,
          webgl.num_reports(uint32_t(-1))
         );

  fclose(file);
}

int main(int argc, char *argv[])
{
  enum { max_input_line_size = 4096 };
  char line[max_input_line_size];
  FILE *input = NULL;

  if (argc <= 1 || argc >= 4 || !strcmp(argv[1], "--help") || !strcmp(argv[1], "-h"))
  {
    fprintf(stderr, "\
This tool processes a crashdata .csv file from crash_analysis and appends to\n\
gfx-stats-*.csv files in the current working directory, containing statistics\n\
of success rates of gfx features. If these gfx-stats-*.csv don't exist, they\n\
are created with appropriate column headers.\n\
\n\
Usage:\n\
  %s YYYYMMDD-pub-crashdata.csv\n\
or\n\
  %s --stdin-with-date YYYYMMDD\n\
\n\
Example:\n\
  %s 20120202-pub-crashdata.csv\n\
\n\
It is very important that the input csv filename starts with a date in\n\
YYYYMMDD format, as this is how this tool knows what day the crashdata\n\
corresponds to.\n\
\n\
Such crashdata files may be downloaded from the per-day subdirectories of\n\
  https://crash-analysis.mozilla.com/crash_analysis\n\
\n\
The --stdin-with-date variant reads from stdin instead of a file, but\n\
you must still provide a YYYYMMDD date.\n\
\n\
Example:\n\
  %s --stdin-with-date 20120202\n\
\n\
It is legal to have extra trailing characters in the date, they will\n\
just be ignored. A typical usage of this is in a script uncompressing\n\
many .csv.gz files into a pipe and feeding this program with it, so\n\
you can just pass the filename as the date argument.\n\
\n\
Here is such a script you can use:\n\
\n\
#!/bin/sh\n\
\n\
for f in `ls *-pub-crashdata.csv.gz`\n\
do\n\
  echo \"$f\"\n\
  gzip -dc $f | ./gfx-stats --stdin-with-date $f\n\
done\n\
\n", argv[0], argv[0], argv[0], argv[0]);
    return 1;
  }

  bool read_from_stdin = false;
  const char *where_to_read_date_from = argv[1];
  if (!strcmp(argv[1], "--stdin-with-date"))
  {
    if (argc != 3)
    {
      fprintf(stderr, "\
The --stdin-with-date variant wants an extra YYYYMMDD argument. To get help, do:\n\
  %s --help\n", argv[0]);
      return 1;
    }
    read_from_stdin = true;
    where_to_read_date_from = argv[2];
  }

  bool is_date_valid = true;
  if (strlen(where_to_read_date_from) < 8)
  {
    is_date_valid = false;
  }
  else
  {
    for (int i = 0; i < 8; i++)
    {
      if (!isdigit(where_to_read_date_from[i]))
        is_date_valid = false;
    }
  }
  if (!is_date_valid)
  {
    fprintf(stderr, "\
Expected a date in YYYYMMDD format, got %s.\n\
To get help, do:\n\
  %s --help\n", where_to_read_date_from, argv[0]);
    return 1;
  }

  char date[11];
  strncpy(date, where_to_read_date_from, 4);
  date[4] = '-';
  strncpy(date+5, where_to_read_date_from+4, 2);
  date[7] = '-';
  strncpy(date+8, where_to_read_date_from+6, 2);
  date[10] = 0;

  if (read_from_stdin)
    input = stdin;
  else
  {
    input = fopen(argv[1], "r");
    if (!input)
    {
      fprintf(stderr, "Can't open %s for reading\n", argv[1]);
      return 1;
    }
  }

  while(fgets(line, max_input_line_size, input))
  {
    OS os = os_for_line(line);
    webgl.process(line, os);
    layers.process(line, os);
    d3d10layers.process(line, os);
    d3d11layers.process(line, os);
    process_gpuvendors(line, os);
  }

  if (!read_from_stdin)
    fclose(input);

  output(date, bit(OS::win81),       "gfx-stats-win81.csv");
  output(date, bit(OS::win8),        "gfx-stats-win8.csv");
  output(date, bit(OS::win7),        "gfx-stats-win7.csv");
  output(date, bit(OS::winvista),    "gfx-stats-winvista.csv");
  output(date, bit(OS::winxp),       "gfx-stats-winxp.csv");
  output(date, bit(OS::mac),         "gfx-stats-mac.csv");
  output(date, bit(OS::gnulinux),    "gfx-stats-gnulinux.csv");
  output(date, bit(OS::android),     "gfx-stats-android.csv");
  output(date, (bit(OS::win81))
             | (bit(OS::win8))
             | (bit(OS::win7))
             | (bit(OS::winvista))
             | (bit(OS::winxp))
             | (bit(OS::win_other)), "gfx-stats-win-all.csv");
  output(date, uint32_t(-1), "gfx-stats-all.csv");
  output_webgl_attempt_percentage(date, uint32_t(-1), "gfx-stats-webgl-attempts.csv");
  output_os_market_share(date, "gfx-stats-os-market-share.csv");
  output_num_reports(date, "gfx-stats-num-reports.csv");
  output_desktop_gpuvendor_market_share(date, "gfx-stats-desktop-gpuvendor-market-share.csv");
  output_android_gpuvendor_market_share(date, "gfx-stats-android-gpuvendor-market-share.csv");
  return 0;
}
