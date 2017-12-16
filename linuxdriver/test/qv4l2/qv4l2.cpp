
#include "qv4l2.h"

#include <qimage.h>
#include <qpixmap.h>
#include <qtoolbar.h>
#include <qtoolbutton.h>
#include <qpopupmenu.h>
#include <qmenubar.h>
#include <qfile.h>
#include <qfiledialog.h>
#include <qstatusbar.h>
#include <qmessagebox.h>
#include <qapplication.h>
#include <qaccel.h>
#include <qlineedit.h>
#include <qvalidator.h>
#include <qlayout.h>
#include <qvbox.h>
#include <qhbox.h>
#include <qlabel.h>
#include <qslider.h>
#include <qspinbox.h>
#include <qcombobox.h>
#include <qcheckbox.h>
#include <qpushbutton.h>
#include <qpainter.h>
#include <qpaintdevicemetrics.h>
#include <qwhatsthis.h>

#include <fcntl.h>
#include <sys/ioctl.h>
#include <errno.h>

#include "fileopen.xpm"

ApplicationWindow::ApplicationWindow()
    : QMainWindow( 0, "V4L2 main window", WDestructiveClose | WGroupLeader )
{
    QPixmap openIcon, saveIcon;

    fd = -1;

    sigMapper = NULL;
    QToolBar * fileTools = new QToolBar( this, "file operations" );
    fileTools->setLabel( "File Operations" );

    openIcon = QPixmap( fileopen );
    QToolButton * fileOpen
	= new QToolButton( openIcon, "Open File", QString::null,
			   this, SLOT(choose()), fileTools, "open file" );

    (void)QWhatsThis::whatsThisButton( fileTools );

    const char * fileOpenText = "<p><img source=\"fileopen\"> "
		 "Click this button to open a <em>new v4l device</em>.<br>"
		 "You can also select the <b>Open</b> command "
		 "from the <b>File</b> menu.</p>";

    QWhatsThis::add( fileOpen, fileOpenText );

    QMimeSourceFactory::defaultFactory()->setPixmap( "fileopen", openIcon );

    QPopupMenu * file = new QPopupMenu( this );
    menuBar()->insertItem( "&File", file );


    int id;
    id = file->insertItem( openIcon, "&Open...",
			   this, SLOT(choose()), CTRL+Key_O );
    file->setWhatsThis( id, fileOpenText );

    file->insertSeparator();

    file->insertItem( "&Close", this, SLOT(close()), CTRL+Key_W );

    file->insertItem( "&Quit", qApp, SLOT( closeAllWindows() ), CTRL+Key_Q );

    menuBar()->insertSeparator();

    QPopupMenu * help = new QPopupMenu( this );
    menuBar()->insertItem( "&Help", help );

    help->insertItem( "&About", this, SLOT(about()), Key_F1 );
    help->insertItem( "What's &This", this, SLOT(whatsThis()), SHIFT+Key_F1 );

    statusBar()->message( "Ready", 2000 );

    tabs = new QTabWidget(this);
    tabs->setMargin(3);

    //resize( 450, 600 );
}


ApplicationWindow::~ApplicationWindow()
{
	if (fd >= 0) ::close(fd);
}


void ApplicationWindow::setDevice(const QString &device)
{
	if (fd >= 0) ::close(fd);
	while (QWidget *page = tabs->page(0)) {
		tabs->removePage(page);
		delete page;
	}
	delete tabs;
	delete sigMapper;
	tabs = new QTabWidget(this);
	tabs->setMargin(3);
	sigMapper = new QSignalMapper(this);
	connect(sigMapper, SIGNAL(mapped(int)), this, SLOT(ctrlAction(int)));
	ctrlMap.clear();
	widgetMap.clear();
	classMap.clear();
	audioInput = NULL;

	fd = ::open(device, O_RDONLY);
	if (fd >= 0) {
		addGeneralTab();
		addTabs();
	}
	if (QWidget *current = tabs->currentPage()) {
		current->show();
	}
	tabs->show();
	tabs->setFocus();
	setCentralWidget(tabs);
}

void ApplicationWindow::addGeneralTab()
{
	int cnt = 0;
	QVBox *vbox = new QVBox(tabs);
	QGrid *grid = new QGrid(4, vbox);
	grid->setSpacing(3);
	tabs->addTab(vbox, "General");

	struct v4l2_input vin;
	memset(&vin, 0, sizeof(vin));
	if (ioctl(fd, VIDIOC_ENUMINPUT, &vin) >= 0) {
		QLabel *label = new QLabel("Input", grid);
		label->setAlignment(Qt::AlignRight);
		QComboBox *combo = new QComboBox(grid);
		while (ioctl(fd, VIDIOC_ENUMINPUT, &vin) >= 0) {
			combo->insertItem((char *)vin.name);
			vin.index++;
		}
		connect(combo, SIGNAL(activated(int)), SLOT(inputChanged(int)));
		cnt++;
	}

	struct v4l2_output vout;
	memset(&vout, 0, sizeof(vout));
	if (ioctl(fd, VIDIOC_ENUMOUTPUT, &vout) >= 0) {
		QLabel *label = new QLabel("Output", grid);
		label->setAlignment(Qt::AlignRight);
		QComboBox *combo = new QComboBox(grid);
		while (ioctl(fd, VIDIOC_ENUMOUTPUT, &vout) >= 0) {
			combo->insertItem((char *)vout.name);
			vout.index++;
		}
		connect(combo, SIGNAL(activated(int)), SLOT(outputChanged(int)));
		cnt++;
	}

	struct v4l2_audio vaudio;
	memset(&vaudio, 0, sizeof(vaudio));
	if (ioctl(fd, VIDIOC_ENUMAUDIO, &vaudio) >= 0) {
		QLabel *label = new QLabel("Input Audio", grid);
		label->setAlignment(Qt::AlignRight);
		audioInput = new QComboBox(grid);
		vaudio.index = 0;
		while (ioctl(fd, VIDIOC_ENUMAUDIO, &vaudio) >= 0) {
			audioInput->insertItem((char *)vaudio.name);
			vaudio.index++;
		}
		connect(audioInput, SIGNAL(activated(int)), SLOT(inputAudioChanged(int)));
		cnt++;
	}

	struct v4l2_audioout vaudioout;
	memset(&vaudioout, 0, sizeof(vaudioout));
	if (ioctl(fd, VIDIOC_ENUMAUDOUT, &vaudioout) >= 0) {
		QLabel *label = new QLabel("Output Audio", grid);
		label->setAlignment(Qt::AlignRight);
		QComboBox *combo = new QComboBox(grid);
		while (ioctl(fd, VIDIOC_ENUMAUDOUT, &vaudioout) >= 0) {
			combo->insertItem((char *)vaudioout.name);
			vaudioout.index++;
		}
		connect(combo, SIGNAL(activated(int)), SLOT(outputAudioChanged(int)));
		cnt++;
	}

	if (cnt & 1) {
		new QWidget(grid);
		new QWidget(grid);
	}
	QWidget *stretch = new QWidget(grid);
	stretch->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Ignored);
}

void ApplicationWindow::addTabs()
{
	struct v4l2_queryctrl qctrl;
	unsigned ctrl_class;
	unsigned i;
	int id;

	memset(&qctrl, 0, sizeof(qctrl));
	qctrl.id = V4L2_CTRL_FLAG_NEXT_CTRL;
	while (::ioctl(fd, VIDIOC_QUERYCTRL, &qctrl) == 0) {
		if ((qctrl.flags & V4L2_CTRL_FLAG_DISABLED) == 0) {
			ctrlMap[qctrl.id] = qctrl;
			if (qctrl.type != V4L2_CTRL_TYPE_CTRL_CLASS)
				classMap[V4L2_CTRL_ID2CLASS(qctrl.id)].push_back(qctrl.id);
		}
		qctrl.id |= V4L2_CTRL_FLAG_NEXT_CTRL;
	}
	if (qctrl.id == V4L2_CTRL_FLAG_NEXT_CTRL) {
		strcpy((char *)qctrl.name, "User Controls");
		qctrl.id = V4L2_CTRL_CLASS_USER | 1;
		qctrl.type = V4L2_CTRL_TYPE_CTRL_CLASS;
		ctrlMap[qctrl.id] = qctrl;
		for (id = V4L2_CID_USER_BASE; id < V4L2_CID_LASTP1; id++) {
			qctrl.id = id;
			if (::ioctl(fd, VIDIOC_QUERYCTRL, &qctrl))
				continue;
			if (qctrl.flags & V4L2_CTRL_FLAG_DISABLED)
				continue;
			ctrlMap[qctrl.id] = qctrl;
			classMap[V4L2_CTRL_CLASS_USER].push_back(qctrl.id);
		}
		for (qctrl.id = V4L2_CID_PRIVATE_BASE;
				::ioctl(fd, VIDIOC_QUERYCTRL, &qctrl) == 0; qctrl.id++) {
			if (qctrl.flags & V4L2_CTRL_FLAG_DISABLED)
				continue;
			ctrlMap[qctrl.id] = qctrl;
			classMap[V4L2_CTRL_CLASS_USER].push_back(qctrl.id);
		}
	}

	for (ClassMap::iterator iter = classMap.begin(); iter != classMap.end(); ++iter) {
		ctrl_class = V4L2_CTRL_ID2CLASS(iter->second[0]);
		id = ctrl_class | 1;
		const struct v4l2_queryctrl &qctrl = ctrlMap[id];
		QVBox *vbox = new QVBox(tabs);
		QGrid *grid = new QGrid(4, vbox);
		grid->setSpacing(3);
		tabs->addTab(vbox, (char *)qctrl.name);
		for (i = 0; i < iter->second.size(); i++) {
			if (i & 1)
				id = iter->second[(1+iter->second.size()) / 2 + i / 2];
			else
				id = iter->second[i / 2];
			addCtrl(grid, ctrlMap[id]);
		}
		finishGrid(vbox, grid, ctrl_class, i & 1);
	}
}

void ApplicationWindow::finishGrid(QWidget *vbox, QGrid *grid, unsigned ctrl_class, bool odd)
{
	if (odd) {
		new QWidget(grid);
		new QWidget(grid);
	}
	QWidget *stretch = new QWidget(grid);
	stretch->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Ignored);

	QFrame *frame = new QFrame(vbox);
	frame->setFrameShape(QFrame::HLine);
	frame->setFrameShadow(QFrame::Sunken);
	frame->setMargin(3);

	QHBox *hbox = new QHBox(vbox);
	hbox->setSpacing(3);

	QCheckBox *cbox = new QCheckBox("Update on change", hbox);
	widgetMap[ctrl_class | CTRL_UPDATE_ON_CHANGE] = cbox;
	connect(cbox, SIGNAL(clicked()), sigMapper, SLOT(map()));
	sigMapper->setMapping(cbox, ctrl_class | CTRL_UPDATE_ON_CHANGE);

	stretch = new QWidget(hbox);
	stretch->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Fixed);

	QPushButton *defBut = new QPushButton("Set Defaults", hbox);
	widgetMap[ctrl_class | CTRL_DEFAULTS] = defBut;
	connect(defBut, SIGNAL(clicked()), sigMapper, SLOT(map()));
	sigMapper->setMapping(defBut, ctrl_class | CTRL_DEFAULTS);

	QPushButton *refreshBut = new QPushButton("Refresh", hbox);
	widgetMap[ctrl_class | CTRL_REFRESH] = refreshBut;
	connect(refreshBut, SIGNAL(clicked()), sigMapper, SLOT(map()));
	sigMapper->setMapping(refreshBut, ctrl_class | CTRL_REFRESH);

	QPushButton *button = new QPushButton("Update", hbox);
	widgetMap[ctrl_class | CTRL_UPDATE] = button;
	connect(button, SIGNAL(clicked()), sigMapper, SLOT(map()));
	sigMapper->setMapping(button, ctrl_class | CTRL_UPDATE);
	connect(cbox, SIGNAL(toggled(bool)), button, SLOT(setDisabled(bool)));

	cbox->setChecked(ctrl_class == V4L2_CTRL_CLASS_USER);

	refresh(ctrl_class);
}

void ApplicationWindow::addCtrl(QGrid *grid, const struct v4l2_queryctrl &qctrl)
{
	QIntValidator *val;
	QLineEdit *edit;
	QString name((char *)qctrl.name);
	QComboBox *combo;
	struct v4l2_querymenu qmenu;

	QLabel *label = new QLabel(name, grid);
	label->setAlignment(Qt::AlignRight);

	switch (qctrl.type) {
	case V4L2_CTRL_TYPE_INTEGER:
		if (qctrl.flags & V4L2_CTRL_FLAG_SLIDER) {
			widgetMap[qctrl.id] =
				new QSlider(qctrl.minimum, qctrl.maximum,
					qctrl.step, qctrl.default_value,
					Horizontal, grid);
			connect(widgetMap[qctrl.id], SIGNAL(valueChanged(int)),
				sigMapper, SLOT(map()));
			break;
		}

		if (qctrl.maximum - qctrl.minimum <= 255) {
			widgetMap[qctrl.id] =
				new QSpinBox(qctrl.minimum, qctrl.maximum, 1, grid);
			connect(widgetMap[qctrl.id], SIGNAL(valueChanged(int)),
				sigMapper, SLOT(map()));
			break;
		}

		val = new QIntValidator(qctrl.minimum, qctrl.maximum, grid);
		edit = new QLineEdit(grid);
		edit->setValidator(val);
		widgetMap[qctrl.id] = edit;
		connect(widgetMap[qctrl.id], SIGNAL(lostFocus()),
				sigMapper, SLOT(map()));
		connect(widgetMap[qctrl.id], SIGNAL(returnPressed()),
				sigMapper, SLOT(map()));
		break;

	case V4L2_CTRL_TYPE_INTEGER64:
		widgetMap[qctrl.id] = new QLineEdit(grid);
		connect(widgetMap[qctrl.id], SIGNAL(lostFocus()),
				sigMapper, SLOT(map()));
		connect(widgetMap[qctrl.id], SIGNAL(returnPressed()),
				sigMapper, SLOT(map()));
		break;

	case V4L2_CTRL_TYPE_BOOLEAN:
		label->setText("");
		widgetMap[qctrl.id] = new QCheckBox(name, grid);
		connect(widgetMap[qctrl.id], SIGNAL(clicked()),
				sigMapper, SLOT(map()));
		break;

	case V4L2_CTRL_TYPE_BUTTON:
		label->setText("");
		widgetMap[qctrl.id] = new QPushButton((char *)qctrl.name, grid);
		connect(widgetMap[qctrl.id], SIGNAL(clicked()),
				sigMapper, SLOT(map()));
		break;

	case V4L2_CTRL_TYPE_MENU:
		combo = new QComboBox(grid);
		widgetMap[qctrl.id] = combo;
		for (int i = qctrl.minimum; i <= qctrl.maximum; i++) {
			qmenu.id = qctrl.id;
			qmenu.index = i;
			if (::ioctl(fd, VIDIOC_QUERYMENU, &qmenu))
				continue;
			combo->insertItem((char *)qmenu.name);
		}
		connect(widgetMap[qctrl.id], SIGNAL(activated(int)),
				sigMapper, SLOT(map()));
		break;

	default:
		return;
	}
	sigMapper->setMapping(widgetMap[qctrl.id], qctrl.id);
	if (qctrl.flags & (V4L2_CTRL_FLAG_READ_ONLY | V4L2_CTRL_FLAG_INACTIVE))
		widgetMap[qctrl.id]->setDisabled(true);
}

void ApplicationWindow::ctrlAction(int id)
{
	unsigned ctrl_class = V4L2_CTRL_ID2CLASS(id);
	if (ctrl_class == V4L2_CID_PRIVATE_BASE)
		ctrl_class = V4L2_CTRL_CLASS_USER;
	unsigned ctrl = id & 0xffff;
	QCheckBox *cbox = static_cast<QCheckBox *>(widgetMap[ctrl_class | CTRL_UPDATE_ON_CHANGE]);
	bool update = cbox->isChecked();
	bool all = (ctrl == CTRL_UPDATE || (update && ctrl == CTRL_UPDATE_ON_CHANGE));

	if (ctrl == CTRL_DEFAULTS) {
		setDefaults(ctrl_class);
		return;
	}
	if (ctrl == CTRL_REFRESH) {
		refresh(ctrl_class);
		return;
	}
	if (!update && !all && ctrlMap[id].type != V4L2_CTRL_TYPE_BUTTON)
		return;
	if (ctrl_class == V4L2_CTRL_CLASS_USER) {
		if (!all) {
			updateCtrl(id);
			return;
		}
		for (unsigned i = 0; i < classMap[ctrl_class].size(); i++) {
			updateCtrl(classMap[ctrl_class][i]);
		}
		return;
	}
	if (!all) {
		updateCtrl(id);
		return;
	}
	unsigned count = classMap[ctrl_class].size();
	struct v4l2_ext_control *c = new v4l2_ext_control[count];
	struct v4l2_ext_controls ctrls;
	int idx = 0;

	for (unsigned i = 0; i < count; i++) {
		unsigned id = classMap[ctrl_class][i];

		if (ctrlMap[id].flags & (V4L2_CTRL_FLAG_READ_ONLY | V4L2_CTRL_FLAG_INACTIVE))
			continue;
		c[idx].id = id;
		if (ctrlMap[id].type == V4L2_CTRL_TYPE_INTEGER64)
			c[idx].value64 = getVal64(id);
		else
			c[idx].value = getVal(id);
		idx++;
	}
	memset(&ctrls, 0, sizeof(ctrls));
	ctrls.count = idx;
	ctrls.ctrl_class = ctrl_class;
	ctrls.controls = c;
	if (::ioctl(fd, VIDIOC_S_EXT_CTRLS, &ctrls)) {
		int err = errno;

		if (ctrls.error_idx >= ctrls.count) {
			printf("error: %s\n", strerror(err));
		}
		else {
			id = c[ctrls.error_idx].id;
			printf("error %08x (%s): %s\n", id,
					ctrlMap[id].name, strerror(err));
		}
	}
	delete [] c;
	refresh(ctrl_class);
}

long long ApplicationWindow::getVal64(unsigned id)
{
	const v4l2_queryctrl &qctrl = ctrlMap[id];
	QWidget *w = widgetMap[qctrl.id];

	switch (qctrl.type) {
	case V4L2_CTRL_TYPE_INTEGER64:
		return static_cast<QLineEdit *>(w)->text().toLongLong();
	default:
		return 0;
	}
}

int ApplicationWindow::getVal(unsigned id)
{
	const v4l2_queryctrl &qctrl = ctrlMap[id];
	QWidget *w = widgetMap[qctrl.id];
	v4l2_querymenu qmenu;
	int i, idx;

	switch (qctrl.type) {
	case V4L2_CTRL_TYPE_INTEGER:
		if (qctrl.flags & V4L2_CTRL_FLAG_SLIDER) {
			return static_cast<QSlider *>(w)->value();
		}

		if (qctrl.maximum - qctrl.minimum <= 255) {
			return static_cast<QSpinBox *>(w)->value();
		}
		return static_cast<QLineEdit *>(w)->text().toInt();

	case V4L2_CTRL_TYPE_BOOLEAN:
		return static_cast<QCheckBox *>(w)->isChecked();

	case V4L2_CTRL_TYPE_MENU:
		idx = static_cast<QComboBox *>(w)->currentItem();
		for (i = qctrl.minimum; i <= qctrl.maximum; i++) {
			qmenu.id = qctrl.id;
			qmenu.index = i;
			if (::ioctl(fd, VIDIOC_QUERYMENU, &qmenu))
				continue;
			if (idx-- == 0)
				break;
		}
		return i;

	default:
		break;
	}
	return 0;
}

void ApplicationWindow::updateCtrl(unsigned id)
{
	unsigned ctrl_class = V4L2_CTRL_ID2CLASS(id);
	if (ctrl_class == V4L2_CID_PRIVATE_BASE)
		ctrl_class = V4L2_CTRL_CLASS_USER;

	if (ctrlMap[id].flags & (V4L2_CTRL_FLAG_READ_ONLY | V4L2_CTRL_FLAG_INACTIVE))
		return;

	if (ctrl_class == V4L2_CTRL_CLASS_USER) {
		struct v4l2_control c;

		c.id = id;
		c.value = getVal(id);
		if (::ioctl(fd, VIDIOC_S_CTRL, &c)) {
			int err = errno;

			printf("error %08x (%s): %s\n", id,
				ctrlMap[id].name, strerror(err));
		}
		return;
	}
	struct v4l2_ext_control c;
	struct v4l2_ext_controls ctrls;

	memset(&c, 0, sizeof(c));
	memset(&ctrls, 0, sizeof(ctrls));
	c.id = id;
	if (ctrlMap[id].type == V4L2_CTRL_TYPE_INTEGER64)
		c.value64 = getVal64(id);
	else
		c.value = getVal(id);
	ctrls.count = 1;
	ctrls.ctrl_class = ctrl_class;
	ctrls.controls = &c;
	if (::ioctl(fd, VIDIOC_S_EXT_CTRLS, &ctrls)) {
		int err = errno;

		printf("error %08x (%s): %s\n", id,
				ctrlMap[id].name, strerror(err));
	}
	else if (ctrlMap[id].flags & V4L2_CTRL_FLAG_UPDATE)
		refresh(ctrl_class);
	else {
		if (ctrlMap[id].type == V4L2_CTRL_TYPE_INTEGER64)
			setVal64(id, c.value64);
		else
			setVal(id, c.value);
	}
}

void ApplicationWindow::refresh(unsigned ctrl_class)
{
	if (ctrl_class == V4L2_CTRL_CLASS_USER) {
		for (unsigned i = 0; i < classMap[ctrl_class].size(); i++) {
			unsigned id = classMap[ctrl_class][i];

			v4l2_control c;

			c.id = id;
			if (::ioctl(fd, VIDIOC_G_CTRL, &c)) {
				int err = errno;

				printf("error %08x (%s): %s\n", id,
						ctrlMap[id].name, strerror(err));
			}
			setVal(id, c.value);
		}
		return;
	}
	unsigned count = classMap[ctrl_class].size();
	struct v4l2_ext_control *c = new v4l2_ext_control[count];
	struct v4l2_ext_controls ctrls;

	for (unsigned i = 0; i < count; i++) {
		c[i].id = classMap[ctrl_class][i];
	}
	memset(&ctrls, 0, sizeof(ctrls));
	ctrls.count = count;
	ctrls.ctrl_class = ctrl_class;
	ctrls.controls = c;
	if (::ioctl(fd, VIDIOC_G_EXT_CTRLS, &ctrls)) {
		int err = errno;

		if (ctrls.error_idx >= ctrls.count) {
			printf("error: %s\n", strerror(err));
		}
		else {
			unsigned id = c[ctrls.error_idx].id;
			printf("error %08x (%s): %s\n", id,
					ctrlMap[id].name, strerror(err));
		}
	}
	else {
		for (unsigned i = 0; i < ctrls.count; i++) {
			unsigned id = c[i].id;
			if (ctrlMap[id].type == V4L2_CTRL_TYPE_INTEGER64)
				setVal64(id, c[i].value64);
			else
				setVal(id, c[i].value);
			::ioctl(fd, VIDIOC_QUERYCTRL, &ctrlMap[id]);
			widgetMap[id]->setDisabled(ctrlMap[id].flags &
				(V4L2_CTRL_FLAG_READ_ONLY | V4L2_CTRL_FLAG_INACTIVE));
		}
	}
	delete [] c;
}

void ApplicationWindow::setVal(unsigned id, int v)
{
	const v4l2_queryctrl &qctrl = ctrlMap[id];
	v4l2_querymenu qmenu;
	QWidget *w = widgetMap[qctrl.id];
	int i, idx;

	switch (qctrl.type) {
	case V4L2_CTRL_TYPE_INTEGER:
		if (qctrl.flags & V4L2_CTRL_FLAG_SLIDER)
			static_cast<QSlider *>(w)->setValue(v);
		else if (qctrl.maximum - qctrl.minimum <= 255)
			static_cast<QSpinBox *>(w)->setValue(v);
		else
			static_cast<QLineEdit *>(w)->setText(QString::number(v));
		break;

	case V4L2_CTRL_TYPE_BOOLEAN:
		static_cast<QCheckBox *>(w)->setChecked(v);
		break;

	case V4L2_CTRL_TYPE_MENU:
		idx = 0;
		for (i = qctrl.minimum; i <= v; i++) {
			qmenu.id = id;
			qmenu.index = i;
			if (::ioctl(fd, VIDIOC_QUERYMENU, &qmenu))
				continue;
			idx++;
		}
		static_cast<QComboBox *>(w)->setCurrentItem(idx - 1);
		break;
	default:
		break;
	}
}

void ApplicationWindow::setVal64(unsigned id, long long v)
{
	const v4l2_queryctrl &qctrl = ctrlMap[id];
	QWidget *w = widgetMap[qctrl.id];

	switch (qctrl.type) {
	case V4L2_CTRL_TYPE_INTEGER64:
		static_cast<QLineEdit *>(w)->setText(QString::number(v));
		break;
	default:
		break;
	}
}

void ApplicationWindow::setDefaults(unsigned ctrl_class)
{
	for (unsigned i = 0; i < classMap[ctrl_class].size(); i++) {
		unsigned id = classMap[ctrl_class][i];

		if (ctrlMap[id].type != V4L2_CTRL_TYPE_INTEGER64 &&
		    ctrlMap[id].type != V4L2_CTRL_TYPE_BUTTON)
			setVal(id, ctrlMap[id].default_value);
	}
	ctrlAction(ctrl_class | CTRL_UPDATE);
}

void ApplicationWindow::choose()
{
    QString fn = QFileDialog::getOpenFileName( "/dev/v4l", QString::null,
					       this);
    if ( !fn.isEmpty() ) {
	    setDevice(fn);
    }
    else
	statusBar()->message( "Loading aborted", 2000 );
}


void ApplicationWindow::closeEvent( QCloseEvent* ce )
{
	ce->accept();
}

void ApplicationWindow::inputChanged(int input)
{
	ioctl(fd, VIDIOC_S_INPUT, &input);
	struct v4l2_audio vaudio;
	memset(&vaudio, 0, sizeof(vaudio));
	if (audioInput && ioctl(fd, VIDIOC_G_AUDIO, &vaudio) >= 0) {
		audioInput->setCurrentItem(vaudio.index);
	}
}

void ApplicationWindow::outputChanged(int output)
{
	ioctl(fd, VIDIOC_S_OUTPUT, &output);
}

void ApplicationWindow::inputAudioChanged(int input)
{
	struct v4l2_audio vaudio;
	memset(&vaudio, 0, sizeof(vaudio));
	vaudio.index = input;
	ioctl(fd, VIDIOC_S_AUDIO, &vaudio);
}

void ApplicationWindow::outputAudioChanged(int output)
{
	struct v4l2_audioout vaudioout;
	memset(&vaudioout, 0, sizeof(vaudioout));
	vaudioout.index = output;
	ioctl(fd, VIDIOC_S_AUDOUT, &vaudioout);
}

void ApplicationWindow::about()
{
    QMessageBox::about( this, "V4L2 Control Panel",
			"This program allows easy experimenting with video4linux devices.");
}


int main(int argc, char **argv)
{
    QApplication a(argc, argv);
    ApplicationWindow *mw = new ApplicationWindow();
    mw->setCaption( "V4L2 Control Panel" );
    mw->setDevice("/dev/video0");
    mw->show();
    a.connect( &a, SIGNAL(lastWindowClosed()), &a, SLOT(quit()) );
    return a.exec();
}
