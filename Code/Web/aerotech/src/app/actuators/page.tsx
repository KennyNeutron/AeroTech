import AppHeader from "@/components/AppHeader";
import NavTabs from "@/components/NavTabs";

export default function ActuatorsPage() {
  return (
    <main className="min-h-screen bg-brand-50">
      <AppHeader />

      <div className="px-4">
        <NavTabs />
      </div>

      <section className="flex items-center justify-center py-16">
        <h2 className="text-4xl font-bold text-brand-700">Actuators</h2>
      </section>
    </main>
  );
}
