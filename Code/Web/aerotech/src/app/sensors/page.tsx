import AppHeader from "@/components/AppHeader";
import NavTabs from "@/components/NavTabs";

// ------- Mock data (replace with live data later) -------
type WaterLevelCode = 0 | 1 | 2; // 0=Low, 1=Mid, 2=High

type SensorData = {
  ph: number; // pH
  tds: number; // ppm
  tempC: number; // °C
  waterLevel: WaterLevelCode; // 0/1/2 from Arduino
  updatedAt: string; // ISO
};

// Demo values — swap these when you fetch real data.
const data: SensorData = {
  ph: 6.8,
  tds: 850,
  tempC: 22.5,
  waterLevel: 1, // 0=Low, 1=Mid, 2=High
  updatedAt: new Date().toISOString(),
};

// ------- Helpers / thresholds -------
const waterLabel: Record<WaterLevelCode, "Low" | "Mid" | "High"> = {
  0: "Low",
  1: "Mid",
  2: "High",
};

function badge(text: string, tone: "good" | "warn" | "bad") {
  const base =
    "inline-flex items-center rounded-full px-2.5 py-0.5 text-xs font-medium";
  const toneClass =
    tone === "good"
      ? "bg-emerald-50 text-emerald-700 ring-1 ring-emerald-200"
      : tone === "warn"
      ? "bg-amber-50 text-amber-700 ring-1 ring-amber-200"
      : "bg-rose-50 text-rose-700 ring-1 ring-rose-200";
  return <span className={`${base} ${toneClass}`}>{text}</span>;
}

// Very simple ranges — tune to your system’s targets
function statusPH(v: number) {
  if (v < 5.5) return badge("Low", "warn");
  if (v > 7.0) return badge("High", "warn");
  return badge("Normal", "good");
}
function statusTDS(v: number) {
  if (v < 600) return badge("Low", "warn");
  if (v > 1200) return badge("High", "warn");
  return badge("Normal", "good");
}
function statusTemp(v: number) {
  if (v < 18) return badge("Low", "warn");
  if (v > 28) return badge("High", "warn");
  return badge("Normal", "good");
}
function statusWater(code: WaterLevelCode) {
  if (code === 0) return badge("Low", "bad");
  if (code === 1) return badge("Mid", "good");
  return badge("High", "good");
}

export default function SensorsPage() {
  const updated = new Date(data.updatedAt).toLocaleString();

  return (
    <main className="min-h-screen bg-brand-50">
      <AppHeader />
      <div className="px-4">
        <NavTabs />
      </div>

      {/* System status card */}
      <section className="px-4 mt-6">
        <div className="max-w-5xl mx-auto bg-white border border-brand-100 shadow-card rounded-2xl p-4">
          <div className="flex items-center justify-between flex-wrap gap-3">
            <div className="flex items-center gap-2">
              <span className="text-brand-800 font-medium">System Status</span>
            </div>
            <div className="flex items-center gap-3">
              {badge("Healthy", "good")}
              <div className="text-sm text-brand-800/70">
                <span className="mr-1">Updated:</span>
                <span className="font-medium text-brand-800">{updated}</span>
              </div>
            </div>
          </div>
        </div>
      </section>

      {/* 2x2 metric grid */}
      <section className="px-4 mt-6">
        <div className="max-w-5xl mx-auto grid grid-cols-1 md:grid-cols-2 gap-5">
          {/* pH */}
          <MetricCard
            title="pH Level"
            value={data.ph.toFixed(1)}
            unit="pH"
            status={statusPH(data.ph)}
            icon="🧪"
          />

          {/* TDS */}
          <MetricCard
            title="TDS"
            value={data.tds.toFixed(1)}
            unit="ppm"
            status={statusTDS(data.tds)}
            icon="🪨"
          />

          {/* Water Level (categorical) */}
          <MetricCard
            title="Water Level"
            value={waterLabel[data.waterLevel]}
            unit=""
            status={statusWater(data.waterLevel)}
            icon="💧"
          />

          {/* Temperature */}
          <MetricCard
            title="Temperature"
            value={data.tempC.toFixed(1)}
            unit="°C"
            status={statusTemp(data.tempC)}
            icon="🌡️"
          />
        </div>
      </section>

      {/* Targets row */}
      <section className="px-4 mt-6 mb-16">
        <div className="max-w-5xl mx-auto grid grid-cols-1 sm:grid-cols-2 md:grid-cols-4 gap-4">
          <TargetCard label="pH Target" value="6.5 pH" />
          <TargetCard label="TDS Target" value="800 ppm" />
          <TargetCard label="Level Target" value="Mid" />
          <TargetCard label="Temp Target" value="24.0°C" />
        </div>
      </section>
    </main>
  );
}

// ------- Small presentational components -------
function MetricCard(props: {
  title: string;
  value: string | number;
  unit: string;
  status: React.ReactNode;
  icon?: string;
}) {
  const { title, value, unit, status, icon } = props;
  return (
    <div className="bg-white rounded-2xl border border-brand-100 shadow-card p-5">
      <div className="flex items-center justify-between">
        <h3 className="text-brand-700 font-semibold flex items-center gap-2">
          {icon && <span className="text-lg">{icon}</span>}
          {title}
        </h3>
        <div>{status}</div>
      </div>

      <div className="mt-6">
        <div className="text-4xl font-bold text-brand-700">
          {value}
          {unit ? (
            <span className="ml-2 text-base font-medium text-brand-800/70">
              {unit}
            </span>
          ) : null}
        </div>
        {/* secondary line if you ever want it */}
        {/* <div className="text-brand-800/60 mt-1">subtitle</div> */}
      </div>
    </div>
  );
}

function TargetCard(props: { label: string; value: string }) {
  return (
    <div className="bg-white rounded-xl border border-brand-100 shadow-card p-4">
      <div className="text-brand-800/70 text-sm">{props.label}</div>
      <div className="text-brand-900 font-semibold mt-1">{props.value}</div>
    </div>
  );
}
