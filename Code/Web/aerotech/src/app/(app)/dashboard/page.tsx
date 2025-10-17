export default function DashboardPage() {
  return (
    <main className="min-h-screen flex items-center justify-center p-6">
      <div className="w-full max-w-2xl bg-white rounded-2xl shadow-xl p-8">
        <h1 className="text-3xl font-bold text-gray-900">Dashboard</h1>
        <p className="mt-3 text-gray-600">
          You made it! This route is intended to be protected. Middleware will
          redirect unauthenticated users to <code>/login</code>.
        </p>
      </div>
    </main>
  );
}
